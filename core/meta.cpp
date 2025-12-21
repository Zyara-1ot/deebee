#include "meta.hpp"

Meta& meta() {
    static Meta instance;   // initialized on first use
    return instance;
}
