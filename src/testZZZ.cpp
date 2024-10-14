#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <stdexcept>

// Fonction pour calculer la moyenne d'un vecteur
double calculate_mean(const std::vector<uint16_t>& values) {
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

// Fonction pour calculer l'écart type d'un vecteur
double calculate_stddev(const std::vector<uint16_t>& values, double mean) {
    double sum = 0.0;
    for (auto value : values) {
        sum += (value - mean) * (value - mean);
    }
    return std::sqrt(sum / values.size());
}

// Fonction pour calculer le Z-score
std::vector<double> calculate_z_scores(const std::vector<uint16_t>& values) {
    if (values.empty()) {
        throw std::invalid_argument("Le vecteur ne doit pas être vide");
    }

    double mean = calculate_mean(values);
    double stddev = calculate_stddev(values, mean);

    if (stddev == 0) {
        throw std::invalid_argument("L'écart type est zéro, les Z-scores ne peuvent pas être calculés");
    }

    std::vector<double> z_scores;
    z_scores.reserve(values.size());

    for (auto value : values) {
        double z = (value - mean) / stddev;
        z_scores.push_back(z);
    }

    return z_scores;
}

// Exemple d'utilisation
// int main() {
//     std::vector<uint16_t> depth_map = {0, 450, 500, 550, 600, 1000};

//     try {
//         std::vector<double> z_scores = calculate_z_scores(depth_map);

//         for (double z : z_scores) {
//             std::cout << z << std::endl;
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "Erreur: " << e.what() << std::endl;
//     }

//     return 0;
// }