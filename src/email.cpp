// refer:
//   https://curl.haxx.se/libcurl/c/smtp-mail.html
//   https://curl.haxx.se/libcurl/c/smtp-mime.html

#include "email.h"
#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdexcept>
#include <vector>

struct email_smtp_server {
    std::string server;
    email_user user;
    std::string passwd;
};

static email_smtp_server kSmtpServer;

static inline const char *__format_user(const email_user& u, bool with_name = false)
{
    static char buf[1024] = {0};
    if (with_name) {
        snprintf(buf, 1024, "%s <%s>", u.name.c_str(), u.email.c_str());
    } else {
        snprintf(buf, 1024, "<%s>", u.email.c_str());
    }
    return buf;
}

static curl_slist *__email_header_slist(CURL *curl, const email_send_info& info)
{
    curl_slist *headers = nullptr;
    char buf[1024] = {0};
    std::string sep = "";

    // from
    snprintf(buf, 1024, "From: %s", __format_user(kSmtpServer.user, true));
    headers = curl_slist_append(headers, buf);

    // to
    std::string tos = "To: ";
    sep = "";
    for (const auto& u: info.sends) {
        tos = tos + sep + __format_user(u, true);
        sep = ",";
    }
    headers = curl_slist_append(headers, tos.c_str());

    // cc
    std::string ccs = "Cc: ";
    sep = "";
    for (const auto& u: info.ccs) {
        ccs = ccs + sep + __format_user(u, true);
        sep = ",";
    }
    headers = curl_slist_append(headers, ccs.c_str());

    // subject
    snprintf(buf, 1024, "Subject: %s", info.subject.c_str());
    headers = curl_slist_append(headers, buf);

    return headers;
}

static curl_mime *__email_body_mime(CURL *curl, const email_send_info& info)
{
    curl_mimepart *part;
    auto mime = curl_mime_init(curl);
    auto alt = curl_mime_init(curl);

    // html
    part = curl_mime_addpart(alt);
    curl_mime_data(part, info.body.c_str(), CURL_ZERO_TERMINATED);
    curl_mime_type(part, "text/html");

    // create the inline part
    part = curl_mime_addpart(mime);
    curl_mime_subparts(part, alt);
    curl_mime_type(part, "multipart/alternative");
    auto slist = curl_slist_append(NULL, "Content-Disposition: inline");
    curl_mime_headers(part, slist, 1);

    // attachments
    for(const auto& file: info.attachments) {
        part = curl_mime_addpart(mime);
        curl_mime_filedata(part, file.c_str());
    }

    // slist / part / alt need to free ????
    return mime;
}

void email_smtp_init(const std::string &url, const std::string& email,
                            const std::string &passwd, const std::string& nick)
{
    kSmtpServer = email_smtp_server{url, {nick, email}, passwd};
}

void email_smtp_send(const email_send_info& info)
{
    CURL *curl = curl_easy_init();
    if (curl == nullptr) {
        throw std::runtime_error("curl init failed");
    }

    // OPTIONS
    curl_easy_setopt(curl, CURLOPT_URL, kSmtpServer.server.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, kSmtpServer.user.email.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, kSmtpServer.passwd.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, __format_user(kSmtpServer.user));

    curl_slist *recipients = nullptr;
    // to send_to
    for (const auto& u: info.sends) {
        recipients = curl_slist_append(recipients, __format_user(u));
    }
    // cc
    for (const auto& u: info.ccs) {
        recipients = curl_slist_append(recipients, __format_user(u));
    }
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    // HEADERS
    auto headers = __email_header_slist(curl, info);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // BODY
    auto mime = __email_body_mime(curl, info);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    // NOTICE: block api
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    auto res = curl_easy_perform(curl);

    //free all
    curl_slist_free_all(recipients);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_mime_free(mime);

    if (res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }
}

void email_smtp_send(const std::string& nick, const std::string& email,
                     const std::string& subject, const std::string& body)
{
    auto info = email_send_info { {email_user{nick, email}}, {}, subject, body, {} };
    email_smtp_send(info);
}
