
#include "traitement.h"

#define MARGE 50

void del_extremum(std::vector<uint16_t>& depth, int width, int height, int max, int min) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (depth[y * width + x] > max + MARGE)
                depth[y * width + x] = max;
            else if (depth[y * width + x] < min - MARGE)
                depth[y * width + x] = min;
        }
    }
}

void apply_mean_filter(std::vector<uint16_t>& depth, int width, int height) {
    std::vector<uint16_t> filtered_depth = depth; // Create a copy to store the filtered values
    int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (depth[y * width + x] == 0) {
                int sum = 0;
                int count = 0;

                for (int k = 0; k < 8; ++k) {
                    int nx = x + dx[k];
                    int ny = y + dy[k];

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        uint16_t neighbor = depth[ny * width + nx];
                        if (neighbor != 0) {
                            sum += neighbor;
                            count++;
                        }
                    }
                }

                if (count > 0) {
                    filtered_depth[y * width + x] = sum / count;
                } else {
                    filtered_depth[y * width + x] = 0;
                }
            }
        }
    }

    depth = filtered_depth; // Update the original depth with the filtered values
}

void min_max(std::vector<uint16_t>& depth, int width, int height,
/*int xmin, int xmax, int ymin, int ymax,*/ int& min, int& max)
{
    max = 0;
    min = 65535;
    // for (int x = 0; x < width; x++)
    // {

    //     for (int y = 0; y < height; y++)
    //     {
    //         if (x < xmin || x > xmax || y < ymin || y > ymax)
    //             depth[y * width + x] = 0;
    //     }
    // }
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (depth[y * width + x] < min)
                min = depth[y * width + x];
            else if (depth[y * width + x] > max)
                max = depth[y * width + x];
        }
    }
}

double distance(int x1, int y1, int x2, int y2) {
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Function to check if a point is too close to any of the existing points
bool isTooClose(int x, int y, const std::vector<std::pair<int, int>>& points, double minDist) {
    for (const auto& point : points) {
        if (distance(x, y, point.first, point.second) < minDist) {
            return true;
        }
    }
    return false;
}

// Function to check if a point is within the circle
bool isInCircle(int cx, int cy, int x, int y, int radius) {
    int dx = x - cx;
    int dy = y - cy;
    return (dx * dx + dy * dy) <= (radius * radius);
}

// Function to generate all points within a circle
std::vector<std::pair<int, int>> getCirclePoints(int cx, int cy, int radius) {
    std::vector<std::pair<int, int>> points;
    for (int x = cx - radius; x <= cx + radius; ++x) {
        for (int y = cy - radius; y <= cy + radius; ++y) {
            if (isInCircle(cx, cy, x, y, radius)) {
                points.push_back(std::make_pair(x, y));
            }
        }
    }
    return points;
}

// Function to get random points on the circle's edge
std::vector<std::pair<int, int>> getRandomEdgePoints(int cx, int cy, int radius, int numPoints) {
    std::vector<std::pair<int, int>> edgePoints;
    srand(static_cast<unsigned int>(time(0))); // Seed for randomness

    for (int i = 0; i < numPoints; ++i) {
        double angle = ((double) rand() / RAND_MAX) * 2 * M_PI;
        int x = cx + static_cast<int>(radius * cos(angle));
        int y = cy + static_cast<int>(radius * sin(angle));
        edgePoints.push_back(std::make_pair(x, y));
    }

    return edgePoints;
}

std::vector<std::vector<uint16_t>> dilate(std::vector<std::vector<uint16_t>> trajectory, int width, int height)
{


    int radius = 5;
    int diameter = 2 * radius + 1;
    cv::Mat element = cv::Mat::zeros(diameter, diameter, CV_8UC1);

    cv::Point center(radius, radius);
    for (int y = 0; y < diameter; ++y) {
        for (int x = 0; x < diameter; ++x) {
            if (std::sqrt(std::pow(x - center.x, 2) + std::pow(y - center.y, 2)) <= radius) {
                element.at<uint8_t>(y, x) = 1;
            }
        }
    }


    cv::Mat image = cv::Mat::zeros(height, width, CV_8UC1);
    for (const auto& coord : trajectory) {
        image.at<uint8_t>(coord[1], coord[0]) = 255;
    }
    cv::Mat dilatedImage;
    cv::dilate(image, dilatedImage, element);

    std::vector<std::vector<uint16_t>> dilatedCoordinates;
    for (int y = 0; y < dilatedImage.rows; ++y) {
        for (int x = 0; x < dilatedImage.cols; ++x) {
            if (dilatedImage.at<uint8_t>(y, x) == 255) {
                dilatedCoordinates.push_back({static_cast<uint16_t>(x), static_cast<uint16_t>(y)});
            }
        }
    }
    return dilatedCoordinates;
}

// Recursive function to fill the lake with all neighbors that are smaller than a given value
void fillLake(std::set<std::pair<int, int>>& lake, const std::vector<uint16_t>& depth, int width, int height, int x, int y, int max_depth, std::vector<std::vector<bool>>& visited) {
    // Define the directions for moving in the 8-connected neighborhood
    std::vector<std::pair<int, int>> directions = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},         {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };

    for (const auto& dir : directions) {
        int new_x = x + dir.first;
        int new_y = y + dir.second;

        // Ensure the new position is within bounds and not already visited
        if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height &&
                lake.find({new_x, new_y}) == lake.end())
        {
            int new_depth = depth[new_y * width + new_x];
            if (new_depth >= max_depth) {
                lake.insert({new_x, new_y});
                if (visited[new_y][new_x])
                    fillLake(lake, depth, width, height, new_x, new_y, max_depth, visited);
            }
        }
    }
}

// Helper function for the recursive descent
void gradientDescent(std::vector<std::vector<uint16_t>>& trajectory, const std::vector<uint16_t>& depth,
                        int width, int height, int x, int y,
                        std::vector<std::vector<bool>>& visited,
                        uint16_t isfilled = 0, int nb_point = 2000) {
    if (trajectory.size() > nb_point)
        return;
    // Define the directions for moving in the 8-connected neighborhood
    std::vector<std::pair<int, int>> directions = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},         {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };

    int current_depth = depth[y * width + x];
    bool moved = false;

    int smallx = 0;
    int smally = 0;
    for (const auto& dir : directions) {
        int new_x = x + dir.first;
        int new_y = y + dir.second;

        // Ensure the new position is within bounds
        if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height && !visited[new_y][new_x]) {
            int new_depth = depth[new_y * width + new_x];
            if (new_depth + isfilled >= current_depth) {
                // Move to the new position
                moved = true;
                smallx = new_x;
                smally = new_y;
                current_depth = new_depth;
            }
        }
    }
    if (moved)
    {
        trajectory.push_back({(uint16_t)smallx, (uint16_t)smally});
        visited[smally][smallx] = true;
        gradientDescent(trajectory, depth, width, height, smallx, smally, visited, 0, nb_point);
    }

    // If no move was possible, we have reached a local minimum or plateau
    if (!moved && isfilled < 30) {
        std::set<std::pair<int, int>> lake;
        lake.insert({x, y});
        fillLake(lake, depth, width, height, x, y, depth[y * width + x] + isfilled + 1, visited);
        for (const auto& point : lake)
        {
            if (!visited[(uint16_t)point.second][(uint16_t)point.first])
            {
                visited[(uint16_t)point.second][(uint16_t)point.first] = true;
                trajectory.push_back({(uint16_t)point.first, (uint16_t)point.second});
            }
            gradientDescent(trajectory, depth, width, height, (uint16_t)point.first,
                                (uint16_t)point.second, visited, isfilled + 5, nb_point);
        }
    }
}

std::vector<std::vector<std::vector<uint16_t>>> getTraj(std::vector<uint16_t>& depth,
                                            int width, int height,
                                            /*int xmin, int xmax, int ymin, int ymax,*/
                                            int nb_traj)
{
    std::vector<std::vector<std::vector<uint16_t>>> trajectorys;
    int max_depth = 66000;
    int start_x = 0, start_y = 0;
    
    // Find the starting point (the highest point, the most black, i.e., the max depth)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            if (depth[index] < max_depth/* && !(x < xmin || x > xmax || y < ymin || y > ymax)*/) {
                max_depth = depth[index];
                start_x = x;
                start_y = y;
            }
        }
    }
    
    int radius = 7; // Circle radius

    // Get all points within the circle
    std::vector<std::pair<int, int>> circlePoints = getCirclePoints(start_x, start_y, radius);


    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::vector<std::vector<uint16_t>> t;
    for (const auto& point : circlePoints) {
        visited[point.second][point.first] = true;
        t.push_back({(uint16_t)point.first, (uint16_t)point.second});
    }
    trajectorys.push_back(t);

    // Get random points on the circle's edge
    std::vector<std::pair<int, int>> edgePoints;

    while (edgePoints.size() < nb_traj) {
        double angle = ((double) rand() / RAND_MAX) * 2 * M_PI;
        int x = start_x + static_cast<int>(radius * cos(angle));
        int y = start_y + static_cast<int>(radius * sin(angle));

        if (!isTooClose(x, y, edgePoints, 8)) {
            edgePoints.push_back(std::make_pair(x, y));
        }
    }


    for (const auto& point : edgePoints) {
        for (const auto& point : circlePoints) {
            t.push_back({(uint16_t)point.first, (uint16_t)point.second});
            visited[point.second][point.first];
        }

        std::vector<std::vector<uint16_t>> trajectory;
        gradientDescent(trajectory, depth, width, height, point.first, point.second, visited);
        
        std::cout << trajectory.size() << "\n";
        trajectorys.push_back(trajectory);
    }



    return trajectorys;
}

std::vector<std::vector<uint16_t>> getOneIterTraj(std::vector<uint16_t>& depth,
                                            int width, int height,
                                            /*int xmin, int xmax, int ymin, int ymax,*/
                                            int nb_new_point, bool refresh)
{
    static std::vector<std::vector<uint16_t>> trajectory;
    int max_depth = 66000;
    static int curr_x = 0;
    static int curr_y = 0;
    
    if (refresh)
    {
        trajectory.clear();
    }

    // Find the starting point (the highest point, the most black, i.e., the max depth)
    if (trajectory.size() == 0)
    {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = y * width + x;
                if (depth[index] < max_depth/* && !(x < xmin || x > xmax || y < ymin || y > ymax)*/) {
                    max_depth = depth[index];
                    curr_x = x;
                    curr_y = y;
                }
            }
        }
        // Add the starting point to the trajectory
        trajectory.push_back({(uint16_t)curr_x, (uint16_t)curr_y});
    }
    else
    {
        curr_x = trajectory.back()[0];
        curr_y = trajectory.back()[1];
    }


    // Initialize the visited matrix
    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    visited[curr_y][curr_x] = true;

    // Start the gradient descent
    gradientDescent(trajectory, depth, width, height, curr_x, curr_y,
                    visited, 0, trajectory.size() + nb_new_point - 1);

    return trajectory;
}

std::vector<std::vector<uint16_t>> getFirstpointTraj (std::vector<uint16_t>& depth,
                                            int width, int height,
                                            /*int xmin, int xmax, int ymin, int ymax,*/
                                            int nb_point)
{
    static int iter = 1;
    
    std::vector<std::vector<std::vector<uint16_t>>> trajs = getTraj(depth,
                                            width, height,
                                            /*xmin, xmax, ymin, ymax,*/ 4);



    std::vector<std::vector<uint16_t>> result;
    for (const auto &traj : trajs)
    {

        int size = nb_point * iter;
        if (size > traj.size())
            size = traj.size();
        for (int i = 0; i < size && i < traj.size(); i++)
            result.push_back(traj[i]);
    }


    std::vector<std::vector<uint16_t>> dilatedCoordinates = dilate(result, width, height);

    iter++;
    return dilatedCoordinates;
}

std::vector<std::vector<uint16_t>> getFirstpointFromTraj(std::vector<std::vector<std::vector<uint16_t>>> trajs,
                                            int width, int height, int nb_point)
{
    static int iter = 0;
    
    std::vector<std::vector<uint16_t>> result;
    for (int i = 0; i < trajs.size(); i++)
    {
        if (i == 0)
        {
            for (int ii = 0; ii < trajs[i].size(); ii++)
                result.push_back(trajs[i][ii]);
        }
        else
        {
            int size = nb_point * iter;
            if (size > trajs[i].size())
                size = trajs[i].size();
            for (int ii = 0; ii < size; ii++)
                result.push_back(trajs[i][ii]);
        }
    }

    std::vector<std::vector<uint16_t>> dilatedCoordinates = dilate(result, width, height);

    iter++;
    return dilatedCoordinates;

}

bool isInTrajectory(const std::set<std::pair<uint16_t, uint16_t>>& trajectory, int x, int y)
{
    return trajectory.find({x, y}) != trajectory.end();
}

void showTraj(std::vector<uint16_t>& depth, int width, int height,
                std::vector<std::vector<uint16_t>> trajectory)
{
    for (const auto& point: trajectory)
    {
        int i = point[1] * width + point[0];
        depth[i] = 0;
    }
}