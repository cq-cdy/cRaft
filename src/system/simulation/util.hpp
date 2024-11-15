#pragma once
#include <random>
#include <unistd.h>
using namespace std;

static std::random_device rd;
static std::mt19937 gen(rd());

auto get_random_between(int a, int b) {
    return std::uniform_int_distribution<>(a, b)(gen);
}

void random_sleep(int a, int b) {
    auto sleep_time = get_random_between(a, b);
    sleep(sleep_time);
}
