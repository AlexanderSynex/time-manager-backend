

#include "info/Worker.hpp"
#include <fmt/format.h>
#include <optional>
#include <string_view>
#include <userver/server/handlers/exceptions.hpp>

template <typename ValueType>
std::optional<ValueType>
extract (std::string_view key, const userver::formats::json::Value &body,
         bool (userver::formats::json::Value::*isType) () const noexcept)
{
  if (body.HasMember (key))
    {
      if ((body[key].*isType) ())
        {
          return body[key].As<ValueType> ();
        }
    }
  return std::nullopt;
}

Worker::Info
Worker::extractInfo (const userver::formats::json::Value &request_json)
{
  auto data = Worker::Info{};

  data.name = extract<std::string> (Info::name_key, request_json,
                                    &userver::formats::json::Value::IsString);
  data.surname
      = extract<std::string> (Info::surname_key, request_json,
                              &userver::formats::json::Value::IsString);
  data.patronymic
      = extract<std::string> (Info::patronymic_key, request_json,
                              &userver::formats::json::Value::IsString);
  data.password
      = extract<std::string> (Info::password_key, request_json,
                              &userver::formats::json::Value::IsString);

  return data;
}