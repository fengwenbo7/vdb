#include "hnswlib_index.h"

HNSWLIBIndex::HNSWLIBIndex(int dim,int num_data,int m,int ef_construction){
    hnswlib::L2Space space(dim);
    index_=new hnswlib::HierarchicalNSW<float>(&space,num_data,m,ef_construction);
}

void HNSWLIBIndex::insert_vectors(const std::vector<float>& data,uint64_t label){
    index_->addPoint(data.data(),label);
}

std::pair<std::vector<int64_t>,std::vector<float>> HNSWLIBIndex::search_vectors(const std::vector<float>& query,int k,const roaring_bitmap_t* bitmap,int ef_search){
    index_->setEf(ef_search);

    //位图是否传入
    RoaringBitmapIDFilter* selector=nullptr;
    if(bitmap!=nullptr){
        selector=new RoaringBitmapIDFilter(bitmap);
    }
    auto result = index_->searchKnn(query.data(),k,selector);//searchKnn函数有过滤器的参数

    std::vector<int64_t> indices(k);
    std::vector<float> distances(k);
    for(int i=0;i<k;i++){
        auto item=result.top();
        indices[i]=item.second;
        distances[i]=item.first;
        result.pop();
    }
    return {indices,distances};
}