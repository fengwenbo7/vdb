#pragma once 

#include "faiss_index.h"
#include "vector_database.h"
#include "index_factory.h"
#include "rapidjson/document.h"
#include "httplib.h"
#include <string>

class HttpServer{
public:
    enum class CheckType{
        SEARCH,
        INSERT,
        UPSERT
    };

    HttpServer(const std::string& host,int port,VectorDatabase* vector_db);
    void start();

private:
    void searchHandler(const httplib::Request& req,httplib::Response& res);
    void insertHandler(const httplib::Request& req,httplib::Response& res);
    void upsertHandler(const httplib::Request& req,httplib::Response& res);
    void queryHandler(const httplib::Request& req,httplib::Response& res);

    bool isRequestValid(const rapidjson::Document& json_request,CheckType check_type);
    IndexFactory::IndexType getIndexTypeFromRequest(const rapidjson::Document& json_request);
    void setJsonResponse(const rapidjson::Document& json_request,httplib::Response& res);
    void setErrorJsonResponse(httplib::Response& res,int err_code,const std::string& errorMsg);

    std::string host_;
    int port_;
    VectorDatabase* vector_db_;
    httplib::Server server_;
};