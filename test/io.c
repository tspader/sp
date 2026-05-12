#include "io/io.h"
SP_TEST_MAIN()

#include "io/read.c"
#include "io/write.c"
#include "io/copy.c"
#include "io/mem.c"
#include "io/seeking_reader.c"
#include "io/loose.c"

u64 io_get_num_results(const io_result_t* responses, u64 max) {
  u64 n = 0;
  sp_for(it, max) {
    if (!responses[it].bytes && !responses[it].err && !responses[it].data) break;
    n = it + 1;
  }
  return n;
}

sp_err_t io_mock_reader_read(sp_io_reader_t* r, void* ptr, u64 size, u64* bytes_read) {
  io_mock_reader_t* m = (io_mock_reader_t*)r;
  sp_assert(m->cursor < m->num_results);
  io_result_t* resp = &m->results[m->cursor++];
  u64 n = sp_min(size, resp->bytes);
  if (n && resp->data) {
    sp_mem_copy(ptr, resp->data, n);
  }
  if (bytes_read) *bytes_read = n;
  return resp->err;
}

sp_err_t io_mock_writer_write(sp_io_writer_t* w, const void* ptr, u64 size, u64* bytes_written) {
  io_mock_writer_t* m = (io_mock_writer_t*)w;
  sp_assert(m->cursor < m->num_results);
  io_result_t* resp = &m->results[m->cursor++];
  u64 n = sp_min(size, resp->bytes);
  if (n) {
    sp_assert(m->received_len + n <= sizeof(m->received));
    sp_mem_copy(m->received + m->received_len, ptr, n);
    m->received_len += n;
  }
  if (bytes_written) *bytes_written = n;
  return resp->err;
}


void io_mock_reader_init(io_mock_reader_t* m, const io_result_t* responses, u64 n) {
  *m = sp_zero_s(io_mock_reader_t);
  m->base.read = io_mock_reader_read;
  m->num_results = n;
  sp_for(it, n) m->results[it] = responses[it];
}

void io_mock_writer_init(io_mock_writer_t* m, const io_result_t* responses, u64 n) {
  *m = sp_zero_s(io_mock_writer_t);
  m->base.write = io_mock_writer_write;
  m->num_results = n;
  sp_for(it, n) m->results[it] = responses[it];
}

