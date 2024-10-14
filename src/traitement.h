#pragma once

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <set>
#include <opencv2/opencv.hpp>

void del_extremum(std::vector<uint16_t>& depth, int width, int height, int max, int min);

void apply_mean_filter(std::vector<uint16_t>& depth, int width, int height);

void min_max(std::vector<uint16_t>& depth, int width, int height,/*
int xmin, int xmax, int ymin, int ymax,*/ int& min, int& max);

std::vector<std::vector<std::vector<uint16_t>>> getTraj(std::vector<uint16_t>& depth,
                                            int width, int height,
                                            /*int xmin, int xmax, int ymin, int ymax,*/
                                            int nb_traj=1);

std::vector<std::vector<uint16_t>> getOneIterTraj(std::vector<uint16_t>& depth,
                                            int width, int height,
                                            /*int xmin, int xmax, int ymin, int ymax,*/
                                            int nb_new_point = 10, bool refresh = false);

std::vector<std::vector<uint16_t>> getFirstpointTraj(std::vector<uint16_t>& depth,
                                            int width, int height,
                                            /*int xmin, int xmax, int ymin, int ymax,*/
                                            int nb_point);

std::vector<std::vector<uint16_t>> getFirstpointFromTraj(std::vector<std::vector<std::vector<uint16_t>>> trajs,
                                            int width, int height, int nb_point);

bool isInTrajectory(const std::set<std::pair<uint16_t, uint16_t>>& trajectory, int x, int y);

void showTraj(std::vector<uint16_t>& depth, int width, int height,
                std::vector<std::vector<uint16_t>> trajectory);


uint8_t* process_depth(std::vector<uint16_t> depth_vector);

static bool first = true;