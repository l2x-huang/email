#pragma once

#include <string>
#include <vector>

struct email_user {
    std::string name;
    std::string email;
};

struct email_send_info {
    std::vector<email_user> sends;
    std::vector<email_user> ccs;
    std::string subject;
    std::string body;
    std::vector<std::string> attachments;
};

// url: "smtps://smtp.163.com:465" or "smtp://smtp.163.com:25"
// email: "xx@xx.com"
// passwd or auth code
void email_smtp_init(const std::string &url, const std::string& email,
                     const std::string &passwd,
                     const std::string& nick = "me");

void email_smtp_send(const email_send_info &info);
void email_smtp_send(const std::string& nick, const std::string& email,
                     const std::string& subject, const std::string& body);
