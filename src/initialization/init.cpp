﻿#include "init.h"

#include <Logger.hpp>
#include <functional>
#include <filesystem>
#include <fstream>

#include "SQLProcess.hpp"
#include "manager.h"
#include "networkEndianness.hpp"
#include "input.h"

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qini::INIObject serverIni;
extern qls::Manager serverManager;

namespace qls
{
    void Init::createConfig()
    {
        std::filesystem::create_directory("./config");

        if (!std::filesystem::exists("./config/config.ini"))
        {
            std::ofstream outfile("./config/config.ini");

            qini::INIObject ini;
            ini["server"]["host"] = "0.0.0.0";
            ini["server"]["port"] = std::to_string(55555);

            ini["mysql"]["host"] = "127.0.0.1";
            ini["mysql"]["port"] = std::to_string(3306);
            ini["mysql"]["username"] = "";
            ini["mysql"]["password"] = "";

            ini["ssl"]["certificate_file"] = "certs.pem";
            ini["ssl"]["password"] = "";
            ini["ssl"]["key_file"] = "key.pem";
            ini["ssl"]["dh_file"] = "dh.pem";

            outfile << qini::INIWriter::fastWrite(ini);
        }
        return;
    }

    qini::INIObject Init::readConfig()
    {
        std::ifstream infile("./config/config.ini");
        if (!infile) throw std::runtime_error("can't open file");
        return qini::INIParser::fastParse(infile);
    }

    int init()
    {
#ifdef WIN32
        std::system("chcp 65001"); // Set the console code page to UTF-8
#endif
        serverLogger.info("Server log system started successfully!");

        if (qls::isBigEndianness())
            serverLogger.info("The local endianness of the server is big-endian");
        else
            serverLogger.info("The local endianness of the server is little-endian");

        try
        {
            serverLogger.info("Reading configuration file...");
            serverIni = Init::readConfig();
        }
        catch (const std::exception& e)
        {
            serverLogger.error(std::string(e.what()));
            Init::createConfig();
            serverLogger.error("Please modify the configuration file");
            return -1;
        }

        try
        {
            if (std::stoll(serverIni["mysql"]["port"]) > 65535)
                throw std::logic_error("INI configuration file section: mysql, key: port, the port is too large!");

            if (std::stoll(serverIni["mysql"]["port"]) < 0)
                throw std::logic_error("INI configuration file section: mysql, key: port, the port is too small!");

            // Read cert & key
            {
                {
                    std::ifstream cert(serverIni["ssl"]["certificate_file"]),
                        key(serverIni["ssl"]["key_file"]),
                        dh(serverIni["ssl"]["dh_file"]);

                    serverIni["ssl"]["password"];
                    if (!cert || !key)
                        throw std::logic_error("INI configuration file section: ssl, unable to read files!");
                }

                serverLogger.info("Certificate file path: ", serverIni["ssl"]["certificate_file"]);
                serverLogger.info("Password: ", (serverIni["ssl"]["password"].empty() ? "empty" : serverIni["ssl"]["password"]));
                serverLogger.info("Key file path: ", serverIni["ssl"]["key_file"]);
                serverLogger.info("DH file path: ", serverIni["ssl"]["dh_file"]);

                serverNetwork.set_tls_config([](){
                    std::shared_ptr<asio::ssl::context> ssl_context =
                            std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);

                    // Set SSL parameters
                    ssl_context->set_options(
                        asio::ssl::context::default_workarounds
                        | asio::ssl::context::no_sslv2
                        | asio::ssl::context::no_sslv3
                        | asio::ssl::context::no_tlsv1
                        | asio::ssl::context::no_tlsv1_1
                        | asio::ssl::context::single_dh_use
                    );

                    // Configure SSL context
                    if (!serverIni["ssl"]["password"].empty())
                        ssl_context->set_password_callback(std::bind([](){ return serverIni["ssl"]["password"]; }));
                    ssl_context->use_certificate_chain_file(serverIni["ssl"]["certificate_file"]);
                    ssl_context->use_private_key_file(serverIni["ssl"]["key_file"], asio::ssl::context::pem);
                    return ssl_context;
                });
                serverLogger.info("TLS configuration set successfully");
            }
            
            serverLogger.info("Configuration file read successfully!");
        }
        catch (const std::exception& e)
        {
            serverLogger.error(std::string(e.what()));
            // Init::createConfig();
            serverLogger.error("Please modify the configuration file");
            return -1;
        }

        try
        {
            serverLogger.info("Loading serverManager...");

            serverManager.init();

            serverLogger.info("serverManager loaded successfully!");
        }
        catch (const std::exception& e)
        {
            serverLogger.critical(std::string(e.what()));
            serverLogger.critical("serverManager failed to load!");
            return -1;
        }

        try
        {
            serverLogger.info("Server command line starting...");
            std::thread([](){
                Input input;
                input.init();
                std::string command;
                char buffer[1024] {0};
                while (true)
                {
                    std::cin.getline(buffer, 1023);
                    command = buffer;
                    if (!input.input(command))
                        break;
                }
            }).detach();
            
            serverLogger.info("Server listener starting at address: ", serverIni["server"]["host"], ":", serverIni["server"]["port"]);
            serverNetwork.run(serverIni["server"]["host"], std::stoi(serverIni["server"]["port"]));
            
        }
        catch (const std::exception& e)
        {
            serverLogger.error(std::string(e.what()));
            return -1;
        }

        return 0;
    }
}
