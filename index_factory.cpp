#include "index_factory.h"
#include <faiss/IndexFlat.h>

IndexFactory global_index_factory;

void IndexFactory::init(IndexType type,int dim,int num_data,MetricType metric=MetricType::L2){
    auto faiss_metric=faiss::MetricType::METRIC_L2;
    switch (metric)
    {
    case MetricType::L2:
        faiss_metric=faiss::MetricType::METRIC_L2;
        break;
    case MetricType::IP:
        faiss_metric=faiss::MetricType::METRIC_INNER_PRODUCT;
        break;
    
    default:
        break;
    }
    switch (type)
    {
    case IndexType::FLAT:
        index_map_[type]=new FaissIndex(new faiss::IndexFlat(dim,faiss_metric));
        break;
    case IndexType::HNSW:
        index_map_[type]=new HNSWLIBIndex(dim,num_data,16,200);
        break;
    default:
        break;
    }
}

void* IndexFactory::get_index(IndexType type) const{
    auto it=index_map_.find(type);
    if(it!=index_map_.end()){
        return it->second;
    }
    return nullptr;
}

IndexFactory* getGlobalIndexFactory(){
    return &global_index_factory;
}