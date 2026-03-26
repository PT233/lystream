/**
 * @brief 流数据队列测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "utils/mem/stream_3pdata_deque.h"

#include <atomic>
#include <cstring>
#include <iostream>
#include <thread>

bool run_control = true, end_status = false;

Stream3PDataDeque *data_deque = nullptr;

#define TEST_THREAD 3
#define BUFFER_SIZE 125

void Handle()
{
    Stream3PDataDeque::StreamData stream_data;
    for (auto &data : stream_data.img) { data = new unsigned char[BUFFER_SIZE]; }
    for (auto &len : stream_data.datalen) { len = BUFFER_SIZE; }

    static std::atomic<int> loop_count {0};

    while (run_control) {
        int temp_index = loop_count++;
        if (temp_index == 1000) { run_control = false; }

        sprintf((char *)stream_data.img[0], "this is test, index is %d", temp_index);
        data_deque->PushFront(stream_data);
    }

    for (auto &data : stream_data.img) delete[] data;
}

void Recv()
{
    Stream3PDataDeque::StreamData stream_data;
    for (auto &data : stream_data.img) { data = new unsigned char[BUFFER_SIZE]; }
    for (auto &len : stream_data.datalen) { len = BUFFER_SIZE; }

    while (!end_status) {
        memset(stream_data.img[0], 0, BUFFER_SIZE);
        data_deque->PopBack(&stream_data);
        std::cout << stream_data.img[0] << std::endl;
    }
    for (auto &data : stream_data.img) delete[] data;
}

int main(int argc, char *argv[])
{
    int size[3] = {BUFFER_SIZE, BUFFER_SIZE, BUFFER_SIZE};
    data_deque  = new Stream3PDataDeque(size);

    std::thread th_recv(Recv);

    std::thread th[TEST_THREAD];
    for (int i = 0; i < TEST_THREAD; i++) { th[i] = std::thread(Handle); }

    for (int i = 0; i < TEST_THREAD; i++) { th[i].join(); }

    end_status = true;

    Stream3PDataDeque::StreamData stream_data;
    for (auto &data : stream_data.img) { data = new unsigned char[BUFFER_SIZE]; }
    for (auto &len : stream_data.datalen) { len = BUFFER_SIZE; }
    data_deque->PushFront(stream_data);
    for (auto &data : stream_data.img) delete[] data;

    th_recv.join();

    return 0;
}