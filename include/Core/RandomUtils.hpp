#include <ctime>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

class RandomUtils {
private:
    RandomUtils() = delete;
    ~RandomUtils() = delete;

    static std::random_device rd;
    static std::mt19937 gen;

public:
    // Generate a number in a range
    template <typename T> static T generateInRange(T min, T max) {
        static_assert(std::is_arithmetic<T>::value,
                      "T must be a number type (int, float, etc.)");

        if constexpr (std::is_integral<T>::value) {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(gen);
        } else if constexpr (std::is_floating_point<T>::value) {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(gen);
        }
    }

    // Generate a number from a set
    template <typename T> static T generateFromSet(const std::vector<T> &set) {
        if (set.empty())
            throw std::invalid_argument("Set cannot be empty");

        std::uniform_int_distribution<size_t> dist(0, set.size() - 1);
        return set[dist(gen)];
    }

    // Generate a number with given probabilities
    template <typename T>
    static T generateFromSetWithProb(const std::vector<T> &set,
                                     const std::vector<float> &probabilities) {
        if (set.size() != probabilities.size() || set.empty())
            throw std::invalid_argument("Set and probabilities must have the "
                                        "same size and cannot be empty");

        std::vector<float> cumulative_probs(probabilities.size());
        cumulative_probs[0] = probabilities[0];
        for (size_t i = 1; i < probabilities.size(); i++)
            cumulative_probs[i] = cumulative_probs[i - 1] + probabilities[i];

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float rand_val = dist(gen);

        auto it = std::lower_bound(cumulative_probs.begin(),
                                   cumulative_probs.end(), rand_val);
        size_t index = std::distance(cumulative_probs.begin(), it);
        return set[index];
    }

    static bool chooseWithProb(float probability) {
        if (probability < 0.0f || probability > 1.0f)
            throw std::invalid_argument(
                "Probability must be between 0.0 and 1.0");

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(gen) < probability;
    }
};
