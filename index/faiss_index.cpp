#include "faiss_index.h"
#include <iostream>
#include "faiss/IndexIDMap.h"

FaissIndex::FaissIndex(faiss::Index* index):index_(index){}

void FaissIndex::insert_vectors(const std::vector<float>& data,uint64_t label){
    auto id=static_cast<faiss::idx_t>(label);
    index_->add_with_ids(1,data.data(),&id);
}

void FaissIndex::remove_vectors(const std::vector<faiss::idx_t>& ids){
    //由于id是自定义执行的，因此需要映射回去
    faiss::IndexIDMap* id_map=static_cast<faiss::IndexIDMap*>(index_);
    if(id_map){
        faiss::IDSelectorBatch selector(ids.size(),ids.data());
        index_->remove_ids(selector);
    }else{}
}

std::pair<std::vector<int64_t>,std::vector<float>> FaissIndex::search_vectors(const std::vector<float>& query,int k,const roaring_bitmap_t* bitmap){
    int dim=index_->d;
    int num_queries=query.size()/dim;
    std::vector<faiss::idx_t> indices(num_queries*k);
    std::vector<float> distances(num_queries*k);
    //如果传入了位图，那么将其作为faiss搜索的参数传入函数中
    faiss::SearchParameters search_params;
    RoaringBitmapIDSelector selector(bitmap);
    if(bitmap!=nullptr){
        search_params.sel=&selector;
    }

    index_->search(num_queries,query.data(),k,distances.data(),indices.data(),&search_params);
    return {indices,distances};
}