#include "faiss_index.h"
#include <iostream>

FaissIndex::FaissIndex(faiss::Index* index):index_(index){}

void FaissIndex::insert_vectors(const std::vector<float>& data,uint64_t label){
    auto id=static_cast<faiss::idx_t>(label);
    index_->add_with_ids(1,data.data(),&id);
}

std::pair<std::vector<faiss::idx_t>,std::vector<float>> FaissIndex::search_vectors(const std::vector<float>& query,int k){
    int dim=index_->d;
    int num_queries=query.size()/dim;
    std::vector<faiss::idx_t> indices(num_queries*k);
    std::vector<float> distances(num_queries*k);
    index_->search(num_queries,query.data(),k,distances.data(),indices.data());
    return {indices,distances};
}