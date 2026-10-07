#include "AdministrationService.hpp"

#include <fmt/format.h>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/http/status_code.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/cluster_types.hpp>

#include <userver/crypto/hash.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/server/http/http_method.hpp>

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/io/row_types.hpp>
#include <userver/storages/postgres/null.hpp>
#include <userver/storages/postgres/row.hpp>
#include <worktime_postgres_service/sql_queries.hpp>

using namespace userver;
using namespace userver::formats::json;
using namespace services::control_role;

AdministrationService::AdministrationService (
    const components::ComponentConfig &config,
    const components::ComponentContext &component_context)
    : DataBaseAccessable (component_context),
      userver::server::handlers::HttpHandlerJsonBase::HttpHandlerJsonBase (
          config, component_context)
{
}

Value
AdministrationService::HandleRequestJsonThrow (const HttpRequest &,
                                               const Value &,
                                               RequestContext &) const
{
  return {};
}
