#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

// ===========================================================================
// Eight Kazakhstan Playable Locations
//
// 1. Charyn Canyon (Шарын каньоны) - Valley of Castles, red sandstone
// 2. Trans-Ili Alatau (Іле Алатауы) - High Tian Shan snow peaks, alpine spruce
// 3. Bozzhyra (Бозжыра) - Mangystau white chalk spires & limestone cliffs
// 4. Burabay (Бурабай) - Pearl of Kazakhstan, blue lakes & Okzhetpes granite
// 5. Kolsai Lakes (Көлсай көлдері) - Turquoise alpine lakes, spruce valleys
// 6. Ustyurt Plateau (Үстірт үстірті) - Vast chink escarpments, clay desert
// 7. Katon-Karagay (Катонқарағай) - Altai mountain taiga, highland meadows
// 8. Qazaq Auyl (Қазақ ауылы) - Traditional nomad village with authentic yurts
// ===========================================================================

enum class LocationId
{
    CharynCanyon,
    TransIliAlatau,
    Bozzhyra,
    Burabay,
    KolsaiLakes,
    UstyurtPlateau,
    KatonKaragay,
    QazaqAuyl,
    Count
};

struct LocationInfo
{
    LocationId id;
    const char* nameEn;
    const char* nameKz;
    const char* regionEn;
    const char* regionKz;
    const char* descriptionEn;
    const char* descriptionKz;

    // Thematic color palette
    glm::vec3 groundColor;
    glm::vec3 arenaColor;
    glm::vec3 trackColor;
    glm::vec3 boundaryColor;
    glm::vec3 defaultSkyColor;
    glm::vec3 defaultFogColor;
};

class LocationManager
{
public:
    static const LocationInfo& get(LocationId id);
    static int count() { return static_cast<int>(LocationId::Count); }

    void init();
    void selectLocation(LocationId id) { current_ = id; }
    LocationId selectedLocation() const { return current_; }
    const LocationInfo& currentInfo() const { return get(current_); }

    void next();
    void previous();

private:
    LocationId current_ = LocationId::QazaqAuyl;
};
