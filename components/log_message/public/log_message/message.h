#ifndef COMPONENT_LOG_MESSAGE_PUBLIC_LOG_MESSAGE_MESSAGE_H_
#define COMPONENT_LOG_MESSAGE_PUBLIC_LOG_MESSAGE_MESSAGE_H_

#include <ostream>
#include <string>

namespace pipelines::log_message {

class Message {
 public:
  Message() = default;
  Message(const std::string& pipeline_id, const std::string& id,
          const std::string& body, const std::string& next_id)
      : pipeline_id_(pipeline_id), id_(id), body_(body), next_id_(next_id) {}

  const std::string& get_pipeline_id() const { return pipeline_id_; }
  const std::string& get_id() const { return id_; }
  const std::string& get_body() const { return body_; }
  const std::string& get_next_id() const { return next_id_; }

  bool operator==(const Message& other) const {
    return (pipeline_id_ == other.pipeline_id_) && (id_ == other.id_) &&
           (body_ == other.body_) && (next_id_ == other.next_id_);
  }

  friend std::ostream& operator<<(std::ostream& os, const Message& message) {
    os << "(Pipeline ID: \"" << message.pipeline_id_ << "\", "
       << "ID: \"" << message.id_ << "\", "
       << "Body: \"" << message.body_ << "\", "
       << "Next ID: \"" << message.next_id_ << "\")";
    return os;
  }

 private:
  std::string pipeline_id_;
  std::string id_;
  std::string body_;
  std::string next_id_;
};

}  // namespace pipelines::log_message

#endif  // COMPONENT_LOG_MESSAGE_PUBLIC_LOG_MESSAGE_MESSAGE_H_