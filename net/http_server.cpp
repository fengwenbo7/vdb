#include "http_server.h"
#include "constants.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

HttpServer::HttpServer(const std::string& host,int port,VectorDatabase* vector_db)
:host_(host),port_(port),vector_db_(vector_db){
    server_.Post("/search",[this](const httplib::Request& req,httplib::Response& res){ searchHandler(req,res);});
    server_.Post("/insert",[this](const httplib::Request& req,httplib::Response& res){insertHandler(req,res);});
    server_.Post("/upsert",[this](const httplib::Request& req,httplib::Response& res){upsertHandler(req,res);});
    server_.Post("/query",[this](const httplib::Request& req,httplib::Response& res){queryHandler(req,res);});
}

void HttpServer::start(){
    server_.listen(host_.c_str(),port_);
}

bool HttpServer::isRequestValid(const rapidjson::Document& json_request,CheckType check_type){
    switch (check_type)
    {
    case CheckType::SEARCH:
        return json_request.HasMember(REQUEST_VECTORS)&&json_request.HasMember(REQUEST_K)&&
              (!json_request.HasMember(REQUEST_INDEX_TYPE)||json_request[REQUEST_INDEX_TYPE].IsString());
    case CheckType::INSERT:
        return json_request.HasMember(REQUEST_VECTORS)&&json_request.HasMember(REQUEST_ID)&&
              (!json_request.HasMember(REQUEST_INDEX_TYPE)||json_request[REQUEST_INDEX_TYPE].IsString());
    case CheckType::UPSERT:
        return json_request.HasMember(REQUEST_VECTORS)&&json_request.HasMember(REQUEST_ID)&&
              (!json_request.HasMember(REQUEST_INDEX_TYPE)||json_request[REQUEST_INDEX_TYPE].IsString());
    default:
        return false;
    }
}

IndexFactory::IndexType HttpServer::getIndexTypeFromRequest(const rapidjson::Document& json_request){
    if(json_request.HasMember(REQUEST_INDEX_TYPE)&&json_request[REQUEST_INDEX_TYPE].IsString()){
        auto index_type_str=json_request[REQUEST_INDEX_TYPE].GetString();
        if(index_type_str==INDEX_TYPE_FLAT){
            return IndexFactory::IndexType::FLAT;
        }else if(index_type_str==INDEX_TYPE_HNSW){
            return IndexFactory::IndexType::HNSW;
        }
    }
    return IndexFactory::IndexType::UNKNOWN;
}

void HttpServer::searchHandler(const httplib::Request& req,httplib::Response& res){
    // 解析json请求
    rapidjson::Document json_request;
    json_request.Parse(req.body.c_str());

    // 检查json文档是否为有效对象
    if(!json_request.IsObject()){
        return;
    }

    // 检查请求的合法性
    if(!isRequestValid(json_request,HttpServer::CheckType::SEARCH)){
        return;
    }

    // 获取查询参数
    std::vector<float> query;
    for(const auto& q:json_request[REQUEST_VECTORS].GetArray()){
        query.push_back(q.GetFloat());
    }
    int k=json_request[REQUEST_K].GetInt();

    // 获取请求参数中的索引类型
    IndexFactory::IndexType index_type=getIndexTypeFromRequest(json_request);
    if(index_type==IndexFactory::IndexType::UNKNOWN){
        return;
    }

    // 执行查询
    std::pair<std::vector<int64_t>,std::vector<float>> results=vector_db_->search(json_request);

    // 将结果组装成json
    rapidjson::Document json_response;
    json_response.SetObject();
    rapidjson::Document::AllocatorType& allocator=json_request.GetAllocator();

    rapidjson::Value vectors(rapidjson::kArrayType);
    rapidjson::Value distances(rapidjson::kArrayType);
    for (size_t i = 0; i < results.first.size(); i++)
    {
        /* code */
    }
    

    // 回复响应
    json_response.AddMember(RESPONSE_RETCODE,RESPONSE_RETCODE_SUCCESS,allocator);
    setJsonResponse(json_response,res);
}
void HttpServer::insertHandler(const httplib::Request& req,httplib::Response& res);
void HttpServer::upsertHandler(const httplib::Request& req,httplib::Response& res);
void HttpServer::queryHandler(const httplib::Request& req,httplib::Response& res);

void setJsonResponse(const rapidjson::Document& json_request,httplib::Response& res){
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json_request.Accept(writer);
    res.set_content(buffer.GetString(),RESPONSE_CONTENT_TYPE_JSON);
}

void setErrorJsonResponse(httplib::Response& res,int err_code,const std::string& errorMsg){
    rapidjson::Document json_response;
    json_response.SetObject();
    rapidjson::Document::AllocatorType& allocator=json_response.GetAllocator();
    json_response.AddMember(RESPONSE_RETCODE,err_code,allocator);
    json_response.AddMember(RESPONSE_ERROR_MSG,rapidjson::StringRef(errorMsg.c_str()),allocator);
    setJsonResponse(json_response,res);
}