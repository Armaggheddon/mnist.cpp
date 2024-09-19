#include "data_loaders.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <memory>

Tensor one_hot_encode(int label_idx, int num_classes){
    Tensor one_hot({num_classes});
    one_hot[label_idx] = 1;
    return one_hot;
}

int MNISTDataLoader::num_classes(){
    return 10;
}

MNISTDataLoader::MNISTDataLoader(
    std::string data_path, 
    std::string labels_path,
    bool shuffle,
    bool drop_last
){
    this->m_data_path = data_path;
    this->m_labels_path = labels_path;
    this->m_shuffle = shuffle;
    this->m_drop_last = drop_last;

    this->m_offset = 0;

    this->rng = std::default_random_engine();
}

int MNISTDataLoader::steps_per_epoch(int batch_size){

    if(batch_size <= 0){
        std::cerr << "Batch size must be greater than 0" << std::endl;
        return 1; // TODO raise error
    }

    if(m_drop_last)
        return m_dataset.size() / batch_size;
    else
        return (m_dataset.size() + batch_size - 1) / batch_size;
}

BatchData MNISTDataLoader::get_batch(int batch_size){
    if(static_cast<size_t>(m_offset + batch_size) > m_dataset.size() && m_drop_last){
        new_epoch();
        return BatchData();
    }

    BatchData batch;

    for(int i = m_offset, count = 0; i < m_offset + batch_size; i++, count++){
        
        batch.input_data.emplace_back(m_dataset[i].data);

        batch.targets_idx.emplace_back(m_dataset[i].target);
        batch.targets_one_hot.emplace_back(
            one_hot_encode(m_dataset[i].target, 10)
        );
    }

    m_offset += batch_size;
    
    return batch;
}

void MNISTDataLoader::new_epoch(){
    m_offset = 0;
    if(m_shuffle)
        shuffle();
}

void MNISTDataLoader::load(){
    load_data();
    load_labels();
}

void MNISTDataLoader::shuffle(){
    std::shuffle(m_dataset.begin(), m_dataset.end(), rng);
}

void MNISTDataLoader::load_data(){
    std::ifstream data_file(m_data_path, std::ios::binary);
    if(!data_file.is_open()){
        std::cerr << "Error opening file: " << m_data_path << std::endl;
        return;
    }

    int magic_number = 0;
    int number_of_images = 0;
    int rows = 0;
    int cols = 0;

    // read file metadata
    data_file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
    magic_number = __builtin_bswap32(magic_number);
    data_file.read(reinterpret_cast<char*>(&number_of_images), sizeof(number_of_images));
    number_of_images = __builtin_bswap32(number_of_images);
    data_file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    rows = __builtin_bswap32(rows);
    data_file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
    cols = __builtin_bswap32(cols);

    if(m_dataset.size() != static_cast<size_t>(number_of_images))
        m_dataset.resize(number_of_images);

    for(int i=0; i<number_of_images; i++){
        m_dataset[i].data = Tensor({rows*cols});
        for(int j=0; j<rows*cols; j++){
            unsigned char pixel = 0;
            data_file.read(reinterpret_cast<char*>(&pixel), sizeof(pixel));
            m_dataset[i].data[j] = (static_cast<double>(pixel) / 255.0);
        }
    }

    data_file.close();
}

void MNISTDataLoader::load_labels(){
    std::ifstream labels_file(m_labels_path, std::ios::binary);
    if(!labels_file.is_open()){
        std::cerr << "Error opening file: " << m_labels_path << std::endl;
        return;
    }

    int magic_number = 0;
    int number_of_labels = 0;

    // read file metadata
    labels_file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
    magic_number = __builtin_bswap32(magic_number);
    labels_file.read(reinterpret_cast<char*>(&number_of_labels), sizeof(number_of_labels));
    number_of_labels = __builtin_bswap32(number_of_labels);

    if(m_dataset.size() != static_cast<size_t>(number_of_labels))
        m_dataset.resize(number_of_labels);
    
    for(int i=0; i<number_of_labels; i++){
        unsigned char label = 0;
        labels_file.read(reinterpret_cast<char*>(&label), sizeof(label));
        m_dataset[i].target = label;
    }

    labels_file.close();
}