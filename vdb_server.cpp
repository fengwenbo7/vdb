#include "index_factory.h"

int main(){
    IndexFactory* global_index_factory=getGlobalIndexFactory();
    int dim=256;
    int num_data=1000;
    global_index_factory->init(IndexFactory::IndexType::FLAT,dim);
    global_index_factory->init(IndexFactory::IndexType::HNSW,dim,num_data);
    global_index_factory->init(IndexFactory::IndexType::FILTER);
}