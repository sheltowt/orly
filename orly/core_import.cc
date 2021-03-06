/* <orly/core_import.cc>

   Orly core_import utility.

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

#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <fcntl.h>
#include <syslog.h>

#include <base/log.h>
#include <io/binary_input_only_stream.h>
#include <socket/address.h>
#include <orly/protocol.h>
#include <orly/client/client.h>

using namespace std;

/* Command-line arguments. */
class TCmd final
    : public Base::TLog::TCmd {
  public:

  /* Construct with defaults. */
  TCmd() {}

  /* Construct from argc/argv. */
  TCmd(int argc, char *argv[])
      : ServerAddress(Socket::TAddress::IPv4Loopback, Orly::DefaultPortNumber) {
    Parse(argc, argv, TMeta());
  }

  /* The address where the Orly server can be found. */
  Socket::TAddress ServerAddress;

  /* The session id to use.  If not given, then we'll start a new session. */
  Base::TOpt<Base::TUuid> SessionId;

  /* The minimum number of seconds our session should live after we leave. */
  uint32_t TimeToLive = 0;

  /* The file pattern from which to import. */
  string ImportPattern;

  string ImportPkg;

  /* TODO */
  int64_t NumLoadThreads = 0;

  /* TODO */
  int64_t NumMergeThreads = 0;

  /* TODO */
  int64_t NumSimMerge = 0;

  private:

  /* Our meta-type. */
  class TMeta final
      : public Base::TLog::TCmd::TMeta {
    public:

    /* Registers our fields. */
    TMeta()
        : Base::TLog::TCmd::TMeta("The Orly import utility.") {
      Param(
          &TCmd::ServerAddress, "server_address", Optional, "server_address\0sa\0",
          "The address where the Orly server can be found."
      );
      Param(
          &TCmd::SessionId, "session_id", Optional, "session_id\0sid\0",
          "The session id to use.  If not given, then we'll start a new session."
      );
      Param(
          &TCmd::TimeToLive, "time_to_live", Optional, "time_to_live\0ttl\0",
          "The minimum number of seconds our session should live after we leave."
      );
      Param(
          &TCmd::ImportPattern, "import_pattern", Required, "import_pattern\0",
          "A file pattern from which to import."
      );
      Param(
          &TCmd::ImportPkg, "import_pkg", Required, "import_pkg\0",
          "Name of package to import the data as / to."
        );
      Param(
          &TCmd::NumLoadThreads, "num_load_threads", Required, "num_load_threads\0",
          "The number of files to import at the same time."
      );
      Param(
          &TCmd::NumMergeThreads, "num_merge_threads", Required, "num_merge_threads\0",
          "The number of files to merge at the same time."
      );
      Param(
          &TCmd::NumSimMerge, "num_sim_merge", Required, "num_sim_merge\0",
          "The number of files to be merged into a single file at the same time."
      );
    }

  };  // TCmd::TMeta

};  // TCmd

/* Client object. */
class TClient final
    : public Orly::Client::TClient {
  public:

  /* Construct from command-line arguments. */
  TClient(const TCmd &cmd)
      : Orly::Client::TClient(cmd.ServerAddress, cmd.SessionId, chrono::seconds(cmd.TimeToLive)) {}

  private:

  /* See base class. */
  virtual void OnPovFailed(const Base::TUuid &/*repo_id*/) override {}

  /* See base class. */
  virtual void OnUpdateAccepted(const Base::TUuid &/*repo_id*/, const Base::TUuid &/*tracking_id*/) override {}

  /* See base class. */
  virtual void OnUpdateReplicated(const Base::TUuid &/*repo_id*/, const Base::TUuid &/*tracking_id*/) override {}

  /* See base class. */
  virtual void OnUpdateDurable(const Base::TUuid &/*repo_id*/, const Base::TUuid &/*tracking_id*/) override {}

  /* See base class. */
  virtual void OnUpdateSemiDurable(const Base::TUuid &/*repo_id*/, const Base::TUuid &/*tracking_id*/) override {}

};  // TClient

/* Main. */
int main(int argc, char *argv[]) {
  TCmd cmd(argc, argv);
  Base::TLog log(cmd);
  int result;
  try {
    auto client = make_shared<TClient>(cmd);
    syslog(LOG_INFO, "beginning import mode");
    client->BeginImport();
    syslog(LOG_INFO, "import mode begun");
    try {
      client->ImportCoreVector(cmd.ImportPattern, cmd.ImportPkg, cmd.NumLoadThreads, cmd.NumMergeThreads, cmd.NumSimMerge)->Sync();
    } catch (...) {
      syslog(LOG_INFO, "ending import mode due to exception");
      client->EndImport();
      syslog(LOG_INFO, "import mode ended due to error");
      throw;
    }
    syslog(LOG_INFO, "ending import mode");
    client->EndImport();
    syslog(LOG_INFO, "import mode ended");
    result = EXIT_SUCCESS;
  } catch (const exception &ex) {
    syslog(LOG_ERR, "exception; %s", ex.what());
    cerr << "error: " << ex.what() << endl;
    result = EXIT_FAILURE;
  }
  return result;
}
