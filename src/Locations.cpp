#include "Locations.h"

namespace
{
const LocationInfo kLocations[] = {
    // 1. Charyn Canyon
    {
        LocationId::CharynCanyon,
        "CHARYN CANYON",
        "Шарын каньоны",
        "ALMATY REGION",
        "Алматы облысы",
        "Dramatic red sandstone gorge sculpted by the ancient Charyn River. Famous for the Valley of Castles with towering layered pillars and warm terracotta sands.",
        "Шарын өзенінің бойындағы қызыл құмтасты ғажайып шатқал. Биік мұнаралы 'Қамалдар аңғары' және күнгей шөл ландшафты.",
        glm::vec3(0.68f, 0.36f, 0.20f),  // Ground: Terracotta sandstone
        glm::vec3(0.74f, 0.42f, 0.24f),  // Arena: Warm red earth
        glm::vec3(0.60f, 0.30f, 0.16f),  // Track: Deep canyon clay
        glm::vec3(0.92f, 0.45f, 0.20f),  // Boundary trim: Sunset gold
        glm::vec3(0.60f, 0.70f, 0.88f),  // Sky
        glm::vec3(0.82f, 0.54f, 0.36f)   // Fog: Warm canyon dust haze
    },

    // 2. Trans-Ili Alatau
    {
        LocationId::TransIliAlatau,
        "TRANS-ILI ALATAU",
        "Іле Алатауы",
        "NORTHERN TIAN SHAN",
        "Солтүстік Тянь-Шань",
        "Towering snow-draped alpine summits rising above emerald mountain meadows, fragrant Tien Shan spruce forests, and crystal-clear glacial streams.",
        "Асқар шыңды Іле Алатауының қар басқан жоталары. Жасыл жайлау, шыршалы орман және салқын мөлдір тау самалы.",
        glm::vec3(0.32f, 0.55f, 0.24f),  // Ground: Alpine meadow green
        glm::vec3(0.38f, 0.62f, 0.28f),  // Arena: Fresh pasture
        glm::vec3(0.48f, 0.44f, 0.36f),  // Track: Mountain gravel
        glm::vec3(0.00f, 0.68f, 0.85f),  // Boundary: Kazakh turquoise
        glm::vec3(0.42f, 0.68f, 0.94f),  // Sky: Crisp mountain blue
        glm::vec3(0.55f, 0.75f, 0.95f)   // Fog: Cool alpine mist
    },

    // 3. Bozzhyra
    {
        LocationId::Bozzhyra,
        "BOZZHYRA TRACT",
        "Бозжыра шатқалы",
        "MANGYSTAU UPLAND",
        "Маңғыстау үстірті",
        "Otherworldly surreal white limestone chalk cliffs and the iconic twin fangs of Bozzhyra rising dramatically from vast desert salt flats.",
        "Маңғыстаудың ақ борлы қалдық таулары мен атақты қос азу шыңы. Ежелгі Тетис мұхитының таңғажайып түбі.",
        glm::vec3(0.84f, 0.82f, 0.75f),  // Ground: White chalk / pale marl
        glm::vec3(0.88f, 0.85f, 0.78f),  // Arena: Pale limestone flat
        glm::vec3(0.72f, 0.68f, 0.58f),  // Track: Arid clay path
        glm::vec3(0.85f, 0.78f, 0.52f),  // Boundary: Sandstone gold
        glm::vec3(0.58f, 0.76f, 0.94f),  // Sky: Pale desert azure
        glm::vec3(0.85f, 0.82f, 0.80f)   // Fog: White chalk dust perspective
    },

    // 4. Burabay
    {
        LocationId::Burabay,
        "BURABAY (BOROVOYE)",
        "Бурабай көлі",
        "KOKSHETAU HIGHLANDS",
        "Көкшетау қыраты",
        "The famed 'Pearl of Kazakhstan' featuring dense aromatic pine woods, tranquil sapphire lakes, and iconic weathered granite crags like Okzhetpes.",
        "Қазақстанның інжу-маржаны — Бурабай. Қарағайлы ну орман, көгілдір көл және аты аңызға айналған Оқжетпес жартасы.",
        glm::vec3(0.28f, 0.50f, 0.22f),  // Ground: Pine needle loam & grass
        glm::vec3(0.35f, 0.58f, 0.26f),  // Arena: Lush forest clearing
        glm::vec3(0.52f, 0.45f, 0.32f),  // Track: Forest dirt trail
        glm::vec3(0.20f, 0.55f, 0.85f),  // Boundary: Lake blue
        glm::vec3(0.48f, 0.72f, 0.92f),  // Sky: Fresh pine forest sky
        glm::vec3(0.52f, 0.70f, 0.84f)   // Fog: Lake moisture haze
    },

    // 5. Kolsai Lakes
    {
        LocationId::KolsaiLakes,
        "KOLSAI LAKES",
        "Көлсай көлдері",
        "KUNGEI ALATAU RIDGE",
        "Күнгей Алатау",
        "A cascading string of three pristine turquoise alpine lakes enclosed by steep coniferous Tien Shan spruce mountain slopes and snowfields.",
        "Күнгей Алатаудың көркем шатқалындағы үш көгілдір тау көлі. Қарағай-шыршалы орман және биік тау бөктері.",
        glm::vec3(0.30f, 0.54f, 0.25f),  // Ground: Highland green
        glm::vec3(0.36f, 0.60f, 0.28f),  // Arena: Meadow turf
        glm::vec3(0.44f, 0.40f, 0.32f),  // Track: Dark moist mountain trail
        glm::vec3(0.00f, 0.72f, 0.75f),  // Boundary: Turquoise water
        glm::vec3(0.44f, 0.70f, 0.95f),  // Sky: Pure mountain sky
        glm::vec3(0.48f, 0.74f, 0.85f)   // Fog: Turquoise valley haze
    },

    // 6. Ustyurt Plateau
    {
        LocationId::UstyurtPlateau,
        "USTYURT PLATEAU",
        "Үстірт қыраты",
        "CASPIAN DEPRESSION",
        "Каспий маңы ойпаты",
        "Vast, endless desert tableland bounded by dramatic steep escarpments (chinks), solitary flat-topped buttes, and wide golden steppe horizons.",
        "Каспий мен Арал арасындағы шексіз қырат. Алып 'шыңдар' (тік құламалар), шөл дала және ежелгі керуен жолдары.",
        glm::vec3(0.72f, 0.58f, 0.36f),  // Ground: Ochre desert clay
        glm::vec3(0.76f, 0.64f, 0.40f),  // Arena: Dry plateau steppe
        glm::vec3(0.62f, 0.50f, 0.30f),  // Track: Hard clay road
        glm::vec3(0.95f, 0.70f, 0.20f),  // Boundary: Sun gold
        glm::vec3(0.62f, 0.78f, 0.94f),  // Sky: Vast open horizon
        glm::vec3(0.78f, 0.68f, 0.52f)   // Fog: Dry dust horizon
    },

    // 7. Katon-Karagay
    {
        LocationId::KatonKaragay,
        "KATON-KARAGAY",
        "Катонқарағай",
        "ALTAI MOUNTAINS",
        "Алтай таулары",
        "Wild East Kazakhstan Altai sanctuary of lush Siberian cedar and larch taiga, crystal mountain torrents, and the sacred double-horned Mount Belukha.",
        "Шығыс Қазақстан Алтайының жүрегі. Балқарағайлы қалың тайга, Мұзтау шыңы және мөлдір тау өзендері.",
        glm::vec3(0.26f, 0.48f, 0.22f),  // Ground: Deep Altai meadow green
        glm::vec3(0.32f, 0.55f, 0.26f),  // Arena: Rich pasture
        glm::vec3(0.45f, 0.38f, 0.28f),  // Track: Dark cedar loam
        glm::vec3(0.25f, 0.70f, 0.35f),  // Boundary: Emerald green
        glm::vec3(0.42f, 0.68f, 0.92f),  // Sky: Pure Altai blue
        glm::vec3(0.50f, 0.68f, 0.75f)   // Fog: Forest mountain haze
    },

    // 8. Qazaq Auyl
    {
        LocationId::QazaqAuyl,
        "QAZAQ AUYL",
        "Қазақ ауылы",
        "GREAT STEPPE",
        "Ұлы Дала",
        "Traditional nomadic steppe encampment with authentic white felt yurts, decorated shanyraks, horse corrals, Kazakh tribal banners, and rolling steppe.",
        "Ұлы Даладағы қазақтың дәстүрлі ауылы. Ақ шаңқан киіз үйлер, ою-өрнекті басқұрлар, тұлпарлар байланған желі және еркін дала рухы.",
        glm::vec3(0.56f, 0.62f, 0.28f),  // Ground: Golden feather grass steppe
        glm::vec3(0.65f, 0.72f, 0.34f),  // Arena: Traditional turf
        glm::vec3(0.68f, 0.60f, 0.42f),  // Track: Gallop track
        glm::vec3(0.00f, 0.64f, 0.82f),  // Boundary: Kazakh blue & gold
        glm::vec3(0.52f, 0.74f, 0.95f),  // Sky: Classic steppe sky
        glm::vec3(0.68f, 0.78f, 0.88f)   // Fog: Steppe atmospheric perspective
    }
};
}

const LocationInfo& LocationManager::get(LocationId id)
{
    const int idx = static_cast<int>(id);
    if (idx >= 0 && idx < count())
    {
        return kLocations[idx];
    }
    return kLocations[static_cast<int>(LocationId::QazaqAuyl)];
}

void LocationManager::init()
{
    current_ = LocationId::QazaqAuyl;
}

void LocationManager::next()
{
    int idx = static_cast<int>(current_) + 1;
    if (idx >= count()) idx = 0;
    current_ = static_cast<LocationId>(idx);
}

void LocationManager::previous()
{
    int idx = static_cast<int>(current_) - 1;
    if (idx < 0) idx = count() - 1;
    current_ = static_cast<LocationId>(idx);
}
