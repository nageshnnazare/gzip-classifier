#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

/**
 * @file compressor.hpp
 * @brief Gzip compression wrapper used for Normalized Compression Distance
 * (NCD).
 *
 * Normalized Compression Distance measures similarity between two strings by
 * comparing how much their concatenation compresses relative to each string
 * alone:
 *
 *   NCD(x, y) = (C(xy) - min(C(x), C(y))) / max(C(x), C(y))
 *
 * where C(s) is the byte length of gzip-compressing string @c s.  This class
 * exposes @ref compressedSize and @ref compressString so callers can compute
 * those lengths efficiently.  Under the hood it keeps reusable @c z_stream
 * objects so repeated calls avoid repeated allocator setup in zlib.
 *
 * @note Gzip adds a fixed-format overhead (header, block metadata, CRC32;
 *       typically ~20 bytes).  Compressing strings shorter than that will
 *       yield @c C(s) > |s| even though compression succeeded.  NCD uses
 *       relative sizes, so this expansion on tiny strings does not break the
 *       distance measure — but callers should compress full document text,
 *       not individual short tokens or column headers.
 *
 * @see Jiang et al., "Low-Resource Text Classification with Ultralightweight
 *      Models" (2022)
 */

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <zlib.h>

/**
 * @class Compressor
 * @brief Thread-unsafe gzip compressor backed by long-lived zlib streams.
 *
 * Each instance owns one deflate stream and one inflate stream, initialized
 * once in the constructor and torn down in the destructor.  Individual
 * compress/decompress operations reset their respective stream in place via
 * @c deflateReset / @c inflateReset rather than calling Init/End per call.
 *
 * @note Not thread-safe: share one instance across threads only with external
 *       synchronization, or give each thread its own @ref Compressor.
 */
class Compressor final {
public:
  /** @brief Initialize gzip deflate and inflate streams. @throws
   * std::runtime_error on zlib failure. */
  Compressor();

  /** @brief Release zlib stream resources (@c deflateEnd / @c inflateEnd). */
  ~Compressor();

  Compressor(const Compressor &) = delete;
  Compressor &operator=(const Compressor &) = delete;
  Compressor(Compressor &&) = delete;
  Compressor &operator=(Compressor &&) = delete;

  /**
   * @brief Gzip-compress @p text and return the raw compressed bytes.
   *
   * The returned buffer is the complete gzip payload (header + deflate block
   * + trailer).  Only the bytes of @p text are fed to the compressor; no
   * trailing null terminator is appended.
   *
   * @param text Input string to compress.
   * @return Compressed byte sequence.
   * @throws std::runtime_error if deflate fails or output exceeds zlib limits.
   */
  std::vector<Bytef> compressString(std::string_view text);

  /**
   * @brief Decompress a gzip payload produced by @ref compressString.
   *
   * @param compressed Gzip bytes to inflate.
   * @return Original plaintext.
   * @throws std::runtime_error if the payload is corrupt or output is too
   * large.
   */
  std::string decompressString(const std::vector<Bytef> &compressed);

  /**
   * @brief Return @c C(text): the gzip-compressed byte length of @p text.
   *
   * Convenience wrapper around @ref compressString used when building NCD
   * numerators and denominators.
   */
  std::size_t compressedSize(std::string_view text);

  /**
   * @brief Return @c C(a + b): compressed length of the concatenation.
   *
   * Builds @c a followed by @c b in a temporary buffer, then compresses the
   * result.  Used directly in the NCD formula for a pair of strings.
   */
  std::size_t compressedSize(std::string_view a, std::string_view b);

private:
  z_stream _deflateStream{}; ///< Reused gzip deflate stream.
  z_stream _inflateStream{}; ///< Reused gzip inflate stream.

  /** @brief Allocate and configure @c _deflateStream for gzip output. */
  void initDeflateStream();

  /** @brief Allocate and configure @c _inflateStream for gzip input. */
  void initInflateStream();
};

#endif // COMPRESSOR_HPP
