#pragma once

#include <string>
#include <utility>

enum class Status : unsigned char
{
    TODO,
    IN_PROGRESS,
    DONE
};

class Task
{
  public:
    Task(unsigned int id, std::string description, Status status,
         std::string createdAt, std::string updatedAt)
        : id_{id}, description_{std::move(description)}, status_{status},
          createdAt_{std::move(createdAt)}, updatedAt_{std::move(updatedAt)}
    {
    }

    [[nodiscard]] auto getID() const -> unsigned int { return id_; }
    [[nodiscard]] auto getDescription() const -> std::string
    {
        return description_;
    }
    [[nodiscard]] auto getStatus() const -> Status { return status_; }
    [[nodiscard]] auto getCreationDate() const -> std::string
    {
        return createdAt_;
    }
    [[nodiscard]] auto getUpdationDate() const -> std::string
    {
        return updatedAt_;
    }

    void setDescription(std::string description, std::string updatedAt)
    {
        description_ = std::move(description);
        updatedAt_ = std::move(updatedAt);
    }

    void setStatus(Status status, std::string updatedAt)
    {
        status_ = status;
        updatedAt_ = std::move(updatedAt);
    }

  private:
    std::string createdAt_;
    std::string updatedAt_;
    std::string description_;
    unsigned int id_;
    Status status_;
};