/**
 * @brief 流数据队列测试
 * @date 2022/11/28
 * @author laoyao
 */

#include "utils/mem/stream_data_deque.h"

#include <atomic>
#include <cstring>
#include <iostream>
#include <thread>

bool run_control = true, end_status = false;

StreamDataDeque *data_deque = nullptr;

#define TEST_THREAD 3
#define BUFFER_SIZE 125

void Handle()
{
    StreamDataDeque::StreamData stream_data;
    stream_data.data = new unsigned char[BUFFER_SIZE];

    static std::atomic<int> loop_count {0};

    while (run_control) {
        int temp_index = loop_count++;
        if (temp_index == 1000) { run_control = false; }

        sprintf((char *)stream_data.data, "this is index : %d", temp_index);
        stream_data.datalen = 25;
        data_deque->PushFront(stream_data);
    }

    delete[] stream_data.data;
}

void Recv()
{
    StreamDataDeque::StreamData stream_data;
    stream_data.data = new unsigned char[BUFFER_SIZE];

    while (!end_status) {
        memset(stream_data.data, 0, BUFFER_SIZE);
        data_deque->PopBack(&stream_data);
        std::cout << stream_data.data << std::endl;
    }
    delete[] stream_data.data;
}

int main(int argc, char *argv[])
{
    data_deque = new StreamDataDeque(BUFFER_SIZE);

    std::thread th_recv(Recv);

    std::thread th[TEST_THREAD];
    for (int i = 0; i < TEST_THREAD; i++) { th[i] = std::thread(Handle); }

    for (int i = 0; i < TEST_THREAD; i++) { th[i].join(); }

    end_status = true;

    StreamDataDeque::StreamData stream_data;
    stream_data.data    = new unsigned char[BUFFER_SIZE];
    stream_data.datalen = 25;
    data_deque->PushFront(stream_data);
    delete[] stream_data.data;

    th_recv.join();

    return 0;
}