#include "scalar_storage.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

ScalarStorage::ScalarStorage(const std::string& db_path){
    rocksdb::Options options;
    auto status = rocksdb::DB::Open(options,db_path,&db_);
}

ScalarStorage::~ScalarStorage(){
    delete db_;
}

void ScalarStorage::insert_scalar(uint64_t id,const rapidjson::Document& data){
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    data.Accept(writer);

    auto status=db_->Put(rocksdb::WriteOptions(),std::to_string(id),buffer.GetString());
}

rapidjson::Document ScalarStorage::get_scalar(uint64_t id){
    std::string value;
    auto status=db_->Get(rocksdb::ReadOptions(),std::to_string(id),&value);

    rapidjson::Document data;
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    data.Accept(writer);

    return data;
}