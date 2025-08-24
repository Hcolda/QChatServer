#ifndef RETURN_STATE_MESSAGE_HPP
#define RETURN_STATE_MESSAGE_HPP

#include <Json.h>
#include <string_view>

#include "string_param.hpp"

namespace qls {

/**
 * @brief Creates a JSON object with a state and message.
 * @param state The state of the message ("error", "success", etc.).
 * @param msg The message associated with the state.
 * @return qjson::JObject representing the constructed JSON object.
 */
[[nodiscard]] inline qjson::JObject makeMessage(string_param state,
                                                string_param msg) {
  qjson::JObject json; // Create a JSON object

  // Set "state" and "message" fields in the JSON object
  json["state"] = std::string_view(state);
  json["message"] = std::string_view(msg);

  return json; // Return the constructed JSON object
}

/**
 * @brief Creates an error message JSON object.
 * @param msg The error message.
 * @return qjson::JObject representing the error message JSON object.
 */
[[nodiscard]] inline qjson::JObject makeErrorMessage(string_param msg) {
  return makeMessage(
      "error",
      std::move(msg)); // Use makeMessage to create an error JSON object
}

/**
 * @brief Creates a success message JSON object.
 * @param msg The success message.
 * @return qjson::JObject representing the success message JSON object.
 */
[[nodiscard]] inline qjson::JObject makeSuccessMessage(string_param msg) {
  return makeMessage(
      "success",
      std::move(msg)); // Use makeMessage to create a success JSON object
}

} // namespace qls

#endif // !RETURN_STATE_MESSAGE_HPP
