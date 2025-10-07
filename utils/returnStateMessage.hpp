#ifndef RETURN_STATE_MESSAGE_HPP
#define RETURN_STATE_MESSAGE_HPP

#include <Json.h>
#include <string_view>

namespace qls {

/**
 * @brief Creates a JSON object with a state and message.
 * @param state The state of the message ("error", "success", etc.).
 * @param msg The message associated with the state.
 * @return qjson::JObject representing the constructed JSON object.
 */
[[nodiscard]] inline qjson::JObject makeMessage(std::string_view state,
                                                std::string_view msg) {
  qjson::JObject json; // Create a JSON object

  // Set "state" and "message" fields in the JSON object
  json["state"] = state;
  json["message"] = msg;

  return json; // Return the constructed JSON object
}

/**
 * @brief Creates an error message JSON object.
 * @param msg The error message.
 * @return qjson::JObject representing the error message JSON object.
 */
[[nodiscard]] inline qjson::JObject makeErrorMessage(std::string_view msg) {
  return makeMessage("error",
                     msg); // Use makeMessage to create an error JSON object
}

/**
 * @brief Creates a success message JSON object.
 * @param msg The success message.
 * @return qjson::JObject representing the success message JSON object.
 */
[[nodiscard]] inline qjson::JObject makeSuccessMessage(std::string_view msg) {
  return makeMessage("success",
                     msg); // Use makeMessage to create a success JSON object
}

} // namespace qls

#endif // !RETURN_STATE_MESSAGE_HPP
