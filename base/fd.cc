/* <base/fd.cc>

   Implements <base/fd.h>.

   Copyright 2010-2014 OrlyAtomics, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include <base/fd.h>

#include <poll.h>

#include <strm/bin/in.h>
#include <strm/fd.h>

using namespace Base;

bool TFd::IsReadable(int timeout) const {
  assert(this);
  pollfd p;
  p.fd = OsHandle;
  p.events = POLLIN;
  int result;
  Util::IfLt0(result = poll(&p, 1, timeout));
  return result != 0;
}

const TFd Base::In(STDIN_FILENO), Base::Out(STDOUT_FILENO), Base::Err(STDERR_FILENO);

// TODO: This is a utility that should live in base/
/* Read all the data at fd into one giant buffer in string. Not super efficient, but should be good enough, and
   sufficiently dangerous if the data is coming from an untrustworthy source */
std::string Base::ReadAll(TFd &&fd) {
  std::string out;
  Strm::TFdDefault in_prod(std::move(fd));
  Strm::Bin::TIn in_cons(&in_prod);
  uint8_t buf[4096];
  while (size_t read = in_cons.TryRead(buf, 4096)) {
    out.append(reinterpret_cast<char *>(buf), read);
  }

  return out;
}
