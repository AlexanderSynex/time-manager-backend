#include "services/details/UserService.hpp"
#include "info/Worker.hpp"

#include <cstddef>
#include <fmt/format.h>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/request/request_context.hpp>
#include <userver/server/server.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>

#include <worktime_postgres_service/sql_queries.hpp>

using namespace userver;
namespace pg = userver::storages::postgres;
using namespace userver::formats::json;
using namespace services::control_role::details;

bool
UserService::isValidUser (const JsonData &request) noexcept
{
  if (not request.HasMember (Worker::table_key))
    return false;
  if (not request[Worker::table_key].IsInt ())
    return false;
  return true;
}

UserService::UserService (const components::ComponentContext &context,
                          std::string_view db_service_name)
    : p_db (context.FindComponent<components::Postgres> (db_service_name)
                .GetCluster ())
{
}

/// @return Дискриптор пользователя по json-запросу
std::optional<Worker>
UserService::getWorker (server::request::RequestContext &context) const
{
  auto user_id = context.GetDataOptional<int> (Worker::table_key);
  if (not user_id)
    {
      return std::nullopt;
    }
  return Worker{ static_cast<std::size_t> (*user_id) };
}

/// @return Дискриптор пользователя по json-запросу
std::optional<Worker>
UserService::getWorker (const formats::json::Value &body) const
{
  if (not body[Worker::table_key].IsInt ())
    {
      return std::nullopt;
    }
  return Worker{ static_cast<std::size_t> (
      body[Worker::table_key].As<int> ()) };
}

/// @brief Формируем запрос с данными пользователя по дискриптору
/// @details Возможно добавить кеширование
UserService::JsonData
UserService::getUserInfo (const Worker &user) const
{
  auto trx = db ()->Begin ("finding_user_info",
                           storages::postgres::ClusterHostType::kMaster, {});
  auto res = trx.Execute (worktime_postgres_service::sql::kFindUserInfoById,
                          static_cast<int> (user));
  if (not res.RowsAffected ())
    {
      throw server::handlers::InternalServerError{
        server::handlers::ExternalBody{
            "Unprocessable error while getting user info" }
      };
    }
  auto userInfo = res.Front ();
  auto userData = ValueBuilder{};
  userData[std::string{ Worker::Info::name_key }]
      = userInfo[std::string{ Worker::Info::name_key }].As<std::string> ();
  userData[std::string{ Worker::Info::surname_key }]
      = userInfo[std::string{ Worker::Info::surname_key }].As<std::string> ();
  userData[std::string{ Worker::Info::patronymic_key }]
      = userInfo[std::string{ Worker::Info::patronymic_key }]
            .As<std::string> ();
  return userData.ExtractValue ();
}

bool
UserService::userExists (const Worker &user) const
{
  auto trx = db ()->Begin ("check_user_exist_transaction",
                           storages::postgres::ClusterHostType::kMaster, {});
  auto res = trx.Execute (worktime_postgres_service::sql::kCheckUserExists,
                          static_cast<int> (user));
  trx.Rollback ();
  return res.RowsAffected () > 0;
}
