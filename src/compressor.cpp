/**
 * @file compressor.cpp
 * @brief Gzip compress/decompress implementation using reusable zlib streams.
 *
 * Stream lifecycle
 * ----------------
 * - Constructor: @c deflateInit2 / @c inflateInit2 once per instance.
 * - Each operation: @c deflateReset / @c inflateReset, then one full pass.
 * - Destructor: @c deflateEnd / @c inflateEnd.
 *
 * Gzip vs raw deflate
 * -------------------
 * @c windowBits = MAX_WBITS + 16 tells @c deflateInit2 to emit a gzip wrapper
 * (RFC 1952), matching the Python @c gzip.compress used in the NCD paper.
 * @c inflateInit2 with @c MAX_WBITS + 32 accepts both gzip and zlib payloads.
 *
 * Output buffering
 * ----------------
 * zlib may require multiple @c deflate / @c inflate calls when the caller-
 * supplied output buffer fills before the stream finishes.  Both helpers below
 * grow a @c std::vector<Bytef> geometrically until @c Z_STREAM_END or an error.
 */

#include <compressor.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>

namespace {

/// Initial output capacity: compressed data is usually smaller than input.
constexpr std::size_t kInitialOutputCapacity = 256;

/// Hard cap on decompressed output to guard against corrupt gzip bombs.
constexpr std::size_t kMaxDecompressedBytes = 64 * 1024 * 1024;

/** @brief Turn a zlib return code into a short diagnostic string. */
std::string zlibError(int code) {
  if (const char *msg = zError(code)) {
    return msg;
  }
  return "unknown zlib error (" + std::to_string(code) + ")";
}

} // namespace

Compressor::Compressor() {
  initDeflateStream();
  initInflateStream();
}

Compressor::~Compressor() {
  deflateEnd(&_deflateStream);
  inflateEnd(&_inflateStream);
}

void Compressor::initDeflateStream() {
  _deflateStream.zalloc = Z_NULL;
  _deflateStream.zfree = Z_NULL;
  _deflateStream.opaque = Z_NULL;

  // MAX_WBITS + 16 selects gzip encapsulation instead of raw deflate/zlib.
  const int rc = deflateInit2(&_deflateStream, Z_BEST_COMPRESSION, Z_DEFLATED,
                              MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
  if (rc != Z_OK) {
    throw std::runtime_error("deflateInit2 failed: " + zlibError(rc));
  }
}

void Compressor::initInflateStream() {
  _inflateStream.zalloc = Z_NULL;
  _inflateStream.zfree = Z_NULL;
  _inflateStream.opaque = Z_NULL;

  // MAX_WBITS + 32 enables automatic gzip/zlib header detection on inflate.
  const int rc = inflateInit2(&_inflateStream, MAX_WBITS + 32);
  if (rc != Z_OK) {
    throw std::runtime_error("inflateInit2 failed: " + zlibError(rc));
  }
}

std::vector<Bytef> Compressor::compressString(std::string_view text) {
  const int resetRc = deflateReset(&_deflateStream);
  if (resetRc != Z_OK) {
    throw std::runtime_error("deflateReset failed: " + zlibError(resetRc));
  }

  _deflateStream.avail_in = static_cast<uInt>(text.size());
  _deflateStream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(
      text.data())); // zlib API is C-style; input is read-only.

  std::vector<Bytef> output(std::max(kInitialOutputCapacity, text.size() + 64));
  _deflateStream.avail_out = static_cast<uInt>(output.size());
  _deflateStream.next_out = output.data();

  int rc = deflate(&_deflateStream, Z_FINISH);
  while (rc == Z_OK) {
    const std::size_t produced = output.size() - _deflateStream.avail_out;
    output.resize(output.size() * 2);
    _deflateStream.avail_out = static_cast<uInt>(output.size() - produced);
    _deflateStream.next_out = output.data() + produced;
    rc = deflate(&_deflateStream, Z_FINISH);
  }

  if (rc != Z_STREAM_END) {
    throw std::runtime_error("deflate failed: " + zlibError(rc));
  }

  output.resize(output.size() - _deflateStream.avail_out);
  return output;
}

std::string Compressor::decompressString(const std::vector<Bytef> &compressed) {
  const int resetRc = inflateReset(&_inflateStream);
  if (resetRc != Z_OK) {
    throw std::runtime_error("inflateReset failed: " + zlibError(resetRc));
  }

  _inflateStream.avail_in = static_cast<uInt>(compressed.size());
  _inflateStream.next_in = const_cast<Bytef *>(compressed.data());

  std::vector<Bytef> output(kInitialOutputCapacity);
  _inflateStream.avail_out = static_cast<uInt>(output.size());
  _inflateStream.next_out = output.data();

  int rc = Z_OK;
  while (rc == Z_OK) {
    rc = inflate(&_inflateStream, Z_NO_FLUSH);
    if (rc == Z_STREAM_END) {
      break;
    }
    if (rc != Z_OK) {
      throw std::runtime_error("inflate failed: " + zlibError(rc));
    }

    const std::size_t produced = output.size() - _inflateStream.avail_out;
    if (produced >= kMaxDecompressedBytes) {
      throw std::runtime_error("decompressed output exceeds safety limit");
    }

    output.resize(output.size() * 2);
    _inflateStream.avail_out = static_cast<uInt>(output.size() - produced);
    _inflateStream.next_out = output.data() + produced;
  }

  return std::string(reinterpret_cast<char *>(output.data()),
                     output.size() - _inflateStream.avail_out);
}

std::size_t Compressor::compressedSize(std::string_view text) {
  return compressString(text).size();
}

std::size_t Compressor::compressedSize(std::string_view a, std::string_view b) {
  std::string combined;
  combined.reserve(a.size() + b.size());
  combined.append(a);
  combined.append(b);
  return compressedSize(combined);
}
