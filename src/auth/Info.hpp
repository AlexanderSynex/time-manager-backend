#pragma once

#include <string>

#include <userver/server/auth/user_auth_info.hpp>
#include <vector>

namespace services::auth
{

struct AuthInfo
{
  std::string login;
  userver::server::auth::UserAuthInfo::Ticket token;
  std::vector<std::string> scopes = {};
};

}
