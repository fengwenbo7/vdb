#include "hnswlib_index.h"

HNSWLIBIndex::HNSWLIBIndex(int dim,int num_data,int m,int ef_construction){
    hnswlib::L2Space space(dim);
    index_=new hnswlib::HierarchicalNSW<float>(&space,num_data,m,ef_construction);
}

void HNSWLIBIndex::insert_vectors(const std::vector<float>& data,uint64_t label){
    index_->addPoint(data.data(),label);
}

std::pair<std::vector<long>,std::vector<float>> HNSWLIBIndex::search_vectors(const std::vector<float>& query,int k,int ef_search){
    index_->setEf(ef_search);
    auto result = index_->searchKnn(query.data(),k);

    std::vector<long> indices(k);
    std::vector<float> distances(k);
    for(int i=0;i<k;i++){
        auto item=result.top();
        indices[i]=item.second;
        distances[i]=item.first;
        result.pop();
    }
    return {indices,distances};
}