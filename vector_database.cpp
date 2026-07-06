#include "vector_database.h"
#include "constants.h"
#include "filter_index.h"
#include "faiss_index.h"

VectorDatabase::VectorDatabase(const std::string& db_path):scalar_storage_(db_path){}

void VectorDatabase::upsert(uint64_t id,const rapidjson::Document& data,IndexFactory::IndexType index_type){
    //1.检查标量存储中是否存在给定id的向量
    rapidjson::Document existingData=scalar_storage_.get_scalar(id);

    //2.存在向量的话则先删除向量
    if(existingData.IsObject()){
        void* index=getGlobalIndexFactory()->get_index(index_type);
        switch (index_type) {
        case IndexFactory::IndexType::FLAT:{
            FaissIndex* faiss_index=static_cast<FaissIndex*>(index);
            faiss_index->remove_vectors({static_cast<faiss::idx_t>(id)});
            break;
        }
        case IndexFactory::IndexType::HNSW:{
            HNSWLIBIndex* hnsw_index=static_cast<HNSWLIBIndex*>(index);
            break;
        }
        default:
            break;
        }
    }

    //3.将新向量插入索引
    std::vector<float> new_vector(data["Vectors"].Size());
    for(rapidjson::SizeType i=0;i<data["Vectors"].Size();i++){
        new_vector[i]=data["Vectors"][i].GetFloat();
    }
    void* index=getGlobalIndexFactory()->get_index(index_type);
    switch (index_type) {
        case IndexFactory::IndexType::FLAT:{
            FaissIndex* faiss_index=static_cast<FaissIndex*>(index);
            faiss_index->insert_vectors(new_vector,id);
            break;
        }
        case IndexFactory::IndexType::HNSW:{
            HNSWLIBIndex* hnsw_index=static_cast<HNSWLIBIndex*>(index);
            hnsw_index->insert_vectors(new_vector,id);
            break;
        }
        default:
            break;
    }

    //4.更新标量存储中的向量
    scalar_storage_.insert_scalar(id,data);
}

rapidjson::Document VectorDatabase::query(uint64_t id){
    return scalar_storage_.get_scalar(id);
}

std::pair<std::vector<int64_t>,std::vector<float>> VectorDatabase::search(const rapidjson::Document& json_request){
    // 从JSON请求中获取查询参数
    std::vector<float> query;
    for(const auto& q:json_request[REQUEST_VECTORS].GetArray()){
        query.push_back(q.GetFloat());
    }
    int k=json_request[REQUEST_K].GetInt();
    // 获取请求参数中的索引类型
    IndexFactory::IndexType index_type=IndexFactory::IndexType::UNKNOWN;
    if(json_request.HasMember(REQUEST_INDEX_TYPE)&&json_request[REQUEST_INDEX_TYPE].IsString()){
        auto index_type_str=json_request[REQUEST_INDEX_TYPE].GetString();
        if(index_type_str==INDEX_TYPE_FLAT){
            index_type=IndexFactory::IndexType::FLAT;
        }else if(index_type_str==INDEX_TYPE_HNSW){
            index_type=IndexFactory::IndexType::HNSW;
        }
    }
    // 检查请求中是否包含filter参数
    roaring_bitmap_t* bitmap=nullptr;
    if(json_request.HasMember("filter")&&json_request["filter"].IsObject()){
        const auto& filter=json_request["filter"];
        std::string filter_name=filter["filterName"].GetString();
        std::string op_str=filter["op"].GetString();
        int64_t value=filter["value"].GetInt64();
        FilterIndex::Operation op=(op_str=="=")?FilterIndex::Operation::EQUAL:FilterIndex::Operation::NOT_EQUAL;
        FilterIndex* index=static_cast<FilterIndex*>(getGlobalIndexFactory()->get_index(IndexFactory::IndexType::FILTER));
        auto bitmap=roaring_bitmap_create();
        index->getIntFieldFilterBitmap(filter_name,op,value,bitmap);
    }
    // 使用全局IndexFactory获取索引对象
    auto index=getGlobalIndexFactory()->get_index(index_type);
    // 根据索引类型初始化索引对象并调用search_vectors函数
    std::pair<std::vector<int64_t>,std::vector<float>> results;
    switch (index_type)
    {
    case IndexFactory::IndexType::FLAT:
        FaissIndex* faiss_index=static_cast<FaissIndex*>(index);
        results=faiss_index->search_vectors(query,k,bitmap);
        break;
    case IndexFactory::IndexType::HNSW:
        HNSWLIBIndex* hnsw_index=static_cast<HNSWLIBIndex*>(index);
        results=hnsw_index->search_vectors(query,k,bitmap);
        break;
    default:
        break;
    }

    if(bitmap!=nullptr){
        delete bitmap;
    }
    return results;
}