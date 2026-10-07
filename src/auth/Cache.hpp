#pragma once

#include "Info.hpp"

#include <string_view>
#include <userver/cache/base_postgres_cache.hpp>
#include <userver/crypto/algorithm.hpp>
#include <userver/storages/postgres/io/chrono.hpp>

namespace services::auth::cache
{

inline constexpr std::string_view cacheQuery
    = "SELECT "
      "auth_schema.tokens.token, "
      "auth_schema.tokens.login,"
      "auth_schema.users.scopes, "
      "auth_schema.users.table_id "
      "FROM auth_schema.tokens "
      "LEFT JOIN auth_schema.users ON "
      "auth_schema.users.login=auth_schema.tokens.login";

struct AuthCachePolicy
{
  static constexpr std::string_view kName = "auth-cache";
  static constexpr auto kKeyMember = &AuthInfo::token;
  static constexpr std::string_view kQuery = cacheQuery;
  static constexpr std::string_view kUpdatedField = "last_update";

  using ValueType = AuthInfo;
  using UpdatedFieldType = userver::storages::postgres::TimePointTz;

  using CacheContainer = std::unordered_map<
      userver::server::auth::UserAuthInfo::Ticket, AuthInfo,
      std::hash<userver::server::auth::UserAuthInfo::Ticket>,
      userver::crypto::algorithm::StringsEqualConstTimeComparator>;
};

using AuthCache = userver::components::PostgreCache<AuthCachePolicy>;

}
