#pragma once
#include <vector>
#include "faiss/Index.h"

class FaissIndex{
public:
    FaissIndex(faiss::Index* index);
    void insert_vectors(const std::vector<float>& data,uint64_t label);
    std::pair<std::vector<faiss::idx_t>,std::vector<float>> search_vectors(const std::vector<float>& query,int k);
private:
    faiss::Index* index_;
};