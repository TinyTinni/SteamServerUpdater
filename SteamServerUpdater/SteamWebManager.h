#pragma once

#include <string>

#include <boost/asio.hpp>

// SteamWebManager
// establish connection to steam servers and use the webapi to request information

// currently NOT USED and therefore pretty plain and untested
// main purpose is for future use, when steam maybe updates its api, so the build id can be requested via the web api (version != builid)

class SteamWebManager
{
    boost::asio::io_service m_ioService;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::asio::ip::tcp::resolver::query m_query;
    boost::asio::ip::tcp::socket m_socket;
public:
    SteamWebManager() :
        m_ioService(), m_resolver(m_ioService), m_query("api.steampowered.com", "http"), m_socket(m_ioService)
    {
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(m_query);
        connect(m_socket, endpoint_iterator);
    }
    bool isUpToDate(const std::string& appId, const std::string& buildId)
    {
        using namespace ntw;
        using namespace ntw::ip;
        // send request
        streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << "/ISteamApps/UpToDateCheck/v1?appid=" << appId << "&version=" << buildId << " HTTP/1.0\r\n";
        request_stream << "Host: " << "api.steampowered.com" << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";
        write(m_socket, request);

        // get response
        streambuf response;
        read_until(m_socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            std::cout << "Invalid response\n";
            return "1";
        }
        if (status_code != 200)
        {
            std::cout << "Response returned with status code " << status_code << "\n";
            return "1";
        }

        // Read the response headers, which are terminated by a blank line.
        read_until(m_socket, response, "\r\n\r\n");

        // check for update string
        auto iter = std::find(std::istream_iterator<std::string>(response_stream), std::istream_iterator<std::string>(), "\"up_to_date\":");
        std::cout << *iter++ << " " << *iter << std::endl;

        return (*iter == "true,");
    }

};