#include "vector_database.h"

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
    void* index=getGlobalIndexFactory()->get_index(index_type);
    switch (index_type) {
        case IndexFactory::IndexType::FLAT:{
            FaissIndex* faiss_index=static_cast<FaissIndex*>(index);
            break;
        }
        case IndexFactory::IndexType::HNSW:{
            HNSWLIBIndex* hnsw_index=static_cast<HNSWLIBIndex*>(index);
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