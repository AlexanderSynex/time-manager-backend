#include <userver/clients/dns/component.hpp>
#include <userver/clients/dns/resolver_fwd.hpp>
#include <userver/components/minimal_server_component_list.hpp>

#include <userver/server/handlers/auth/auth_checker_factory.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/postgres_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utest/using_namespace_userver.hpp>
#include <userver/utils/daemon_run.hpp>

// Note: this is for the purposes of tests/samples only
#include <userver/utest/using_namespace_userver.hpp>

#include "auth/Cache.hpp"
#include "services/auth/AuthService.hpp"
#include "services/GeneralComponents.hpp"
#include "services/admin/AdministrationService.hpp"
#include "services/auth/AuthCheckerFactory.hpp"
#include "services/auth/AuthService.hpp"

int
main (int argc, char *argv[])
{
  userver::server::handlers::auth::RegisterAuthCheckerFactory<
      services::auth::AuthCheckerFactory> ();

  auto component_list
      = components::MinimalServerComponentList ()
            .AppendComponentList (services::components::CommonComponents ())
            .Append<services::auth::AuthService> ()
            .Append<services::auth::cache::AuthCache> ()
            .Append<services::control_role::AdministrationService> ();

  return utils::DaemonMain (argc, argv, component_list);
}
