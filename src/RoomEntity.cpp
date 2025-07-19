#include "RoomEntity.h"
#include <algorithm>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>

#define ROOM_WIDTH 6
#define ROOM_HEIGHT 6

TDirtEntity::TDirtEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(TERRAIN_GROUP);
    SetSize({16, 16});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Wall.txt");
}

void TDirtEntity::Update(std::uint32_t delta) {
    auto entityList = NGame::TApp::Instance()->EntityManager().CollisionList(Position(), Size(), DAMAGE_GROUP, Id());

    for (auto& otherId : entityList) {
        auto other = NGame::TApp::Instance()->EntityManager().Entity(otherId);

        if (dynamic_cast<TExplosionEntity*>(other.get())) {
            auto explosionCenter = other->Position() + other->Size() / 2;
            auto ourCenter = Position() + Size() / 2;

            float distance = (ourCenter - explosionCenter).Length();
            if (distance <= other->Size().X) {
                Remove();

                std::size_t rockCount = rand() % 3 + 1;
                while (rockCount--) {
                    NGame::Vec2f speedVector;

                    auto rockEntity = NGame::TApp::Instance()->EntityManager().MakeEntity<TRubbleEntity>();
                    rockEntity->SetPosition(Position());
                    speedVector.X = rand() % 100 - 50;
                    speedVector.Y = rand() % 50;
                    speedVector = speedVector.Normilize() * 100;
                    dynamic_cast<TRubbleEntity*>(rockEntity.get())->SetSpeed(speedVector);
                }

                break;
            }
        }
    }
}

void TDirtEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 1, Position());
}

TGrassEntity::TGrassEntity(NGame::TEntity::TId id)
    : TDirtEntity(id) {
}

void TGrassEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 2, Position());
}

TPlankEntity::TPlankEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(TERRAIN_GROUP);
    SetSize({16, 2});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Plank.txt");
}

void TPlankEntity::Update(std::uint32_t delta) {

}

void TPlankEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 0, Position());
}

TStoneEntity::TStoneEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(TERRAIN_GROUP);
    SetSize({16, 16});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Wall.txt");
}

void TStoneEntity::Update(std::uint32_t delta) {

}

void TStoneEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 0, Position());
}

TLadderEntity::TLadderEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(LADDER_GROUP);
    SetSize({16, 16});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Ladder.txt");
}

void TLadderEntity::Update(std::uint32_t delta) {

}

void TLadderEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 0, Position());

    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Light);
    auto light = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Light.txt");
    NGame::TApp::Instance()->SpriteManager().Draw(light, 1, Position() - (light->Frames[1].Size * 2 / 2) + Size() / 2, {2, 2});
}

TExplosionEntity::TExplosionEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(DAMAGE_GROUP);
    SetSize({64, 64});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Light.txt");
    Alarm_.Set(0, 50);
}

void TExplosionEntity::Update(std::uint32_t delta) {
}

void TExplosionEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 0, Position());
}

void TExplosionEntity::Alarm(NGame::TAlarm::TId id) {
    if (id == 0) {
        Remove();
    }
}

TKeyEntity::TKeyEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(ITEM_GROUP);
    SetSize({16, 16});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Item.txt");
}

void TKeyEntity::Update(std::uint32_t delta) {

}

void TKeyEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 0, Position());
}

TMineEntity::TMineEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetSize({16, 16});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Mine.txt");
    Blinking_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Flash.txt");
}

void TMineEntity::Update(std::uint32_t delta) {
    auto& entityManager = NGame::TApp::Instance()->EntityManager();
    switch (State_) {

    case Dormant:
        if (!entityManager.IsPlaceEmpty(Position() + Size() / 2 - TriggerRadius_ / 2, TriggerRadius_, HERO_GROUP)) {
            Remaining_ = rand() % 5 + 5;
            if (!(Remaining_ % 2)) {
                Remaining_ += 1;
            }
            Alarm_.Set(0, 100);
            State_ = Active;
        }
        break;

    case Active:
        if (!Remaining_) {
            auto explosion = entityManager.MakeEntity<TExplosionEntity>();
            explosion->SetPosition(Position() + Size() / 2 - explosion->Size() / 2 + NGame::Vec2i{0, 4});
            Remove();
        }
        break;
    }

    // Chain detonation
    auto entityList = NGame::TApp::Instance()->EntityManager().CollisionList(Position(), Size(), DAMAGE_GROUP, Id());

    for (auto& otherId : entityList) {
        auto other = NGame::TApp::Instance()->EntityManager().Entity(otherId);

        if (dynamic_cast<TExplosionEntity*>(other.get())) {
            auto explosionCenter = other->Position() + other->Size() / 2;
            auto ourCenter = Position() + Size() / 2;

            float distance = (ourCenter - explosionCenter).Length();
            if (distance <= other->Size().X) {
                auto explosion = entityManager.MakeEntity<TExplosionEntity>();
                explosion->SetPosition(Position() + Size() / 2 - explosion->Size() / 2 + NGame::Vec2i{0, 4});
                Remove();
                break;
            }
        }
    }
    
}

void TMineEntity::Draw() const {
    auto& renderManager = NGame::TApp::Instance()->RenderManager();
    auto& spriteManager = NGame::TApp::Instance()->SpriteManager();

    renderManager.SetLayer(NGame::TRenderManager::Background);
    spriteManager.Draw(Sprite_, 0, Position());

    if (State_ == EState::Active) {
        renderManager.SetLayer(NGame::TRenderManager::EffectsGlow);
        spriteManager.Draw(Blinking_, AlternateBlink_, Position() + (Size() * NGame::Vec2f(0.5f, 0.75f)) - Blinking_->Frames[0].Size, {2, 2});
    }
}

void TMineEntity::Alarm(NGame::TAlarm::TId id) {
    if (id == 0) {
        AlternateBlink_ = !AlternateBlink_;
        if (Remaining_) {
            --Remaining_;
        }
    }
}

TCurseEntity::TCurseEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(ITEM_GROUP);
    SetSize({16, 16});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Item.txt");
}

void TCurseEntity::Update(std::uint32_t delta) {

}

void TCurseEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 0, Position());
}

TSpikeEntity::TSpikeEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(DAMAGE_GROUP);
    SetSize({16, 16});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Spike.txt");
}

void TSpikeEntity::Update(std::uint32_t delta) {

}

void TSpikeEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, IsBloody_, Position());
}

bool TSpikeEntity::IsBloody() const {
    return IsBloody_;
}

void TSpikeEntity::SetBloody(bool value) {
    IsBloody_ = value;
}

TEntranceEntity::TEntranceEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(PASSAGE_GROUP);
    SetSize({16, 16});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Passage.txt");
}

void TEntranceEntity::Update(std::uint32_t delta) {

}

void TEntranceEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 0, Position());
}

TExitEntity::TExitEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetCollisionGroup(PASSAGE_GROUP);
    SetSize({16, 16});
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Passage.txt");
}

void TExitEntity::Update(std::uint32_t delta) {

}

void TExitEntity::Draw() const {
    NGame::TApp::Instance()->RenderManager().SetLayer(NGame::TRenderManager::Background);
    NGame::TApp::Instance()->SpriteManager().Draw(Sprite_, 0, Position());
}

TFloatingTextEntity::TFloatingTextEntity(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    Alarm_.Set(0, 1000);
}

void TFloatingTextEntity::SetText(const std::string& text) {
    Text_ = text;
}

void TFloatingTextEntity::SetColor(NGame::TFontManager::EColor color) {
    Color_ = color;
}

void TFloatingTextEntity::Update(std::uint32_t delta) {
    SetPosition({Position().X, Position().Y - 1});
}

void TFloatingTextEntity::Draw() const {
    auto& renderManager = NGame::TApp::Instance()->RenderManager();
    auto& fontManager = NGame::TApp::Instance()->FontManager();

    renderManager.SetLayer(NGame::TRenderManager::AfterLightEffects);
    fontManager.Draw(Color_, Position(), Text_);
}

void TFloatingTextEntity::Alarm(NGame::TAlarm::TId id) {
    Remove();
}

TRoomEntity::TRoomEntity(NGame::TEntity::TId id) 
    : NGame::TEntity(id) {
    LoadTemplates();
    GenerateLayout();
    Remove();
}

void TRoomEntity::LoadTemplates() {
    auto& fileManager = NGame::TApp::Instance()->FileManager();
    ParseRooms(fileManager.Get("Templates/LR.txt"), LRRooms_);
    ParseRooms(fileManager.Get("Templates/LRD.txt"), LRDRooms_);
    ParseRooms(fileManager.Get("Templates/LRU.txt"), LRURooms_);
    ParseRooms(fileManager.Get("Templates/LRUD.txt"), LRUDRooms_);
    ParseRooms(fileManager.Get("Templates/Any.txt"), AnyRooms_);
}

void TRoomEntity::ParseRooms(const std::string& data, std::vector<TRoom>& rooms) {
    std::istringstream inputStream(data);

    TRoom currentRoom;
    while (ParseRoom(inputStream, currentRoom)) {
        rooms.push_back(currentRoom);
    }
}

bool TRoomEntity::ParseRoom(std::istringstream& stream, TRoom& room) {
    for (std::string line; std::getline(stream, line); ) {
        auto action = NGame::Trim(std::string_view(line));

        if (action == "Room") {
            std::string rows[6];
            std::getline(stream, rows[0]);
            std::getline(stream, rows[1]);
            std::getline(stream, rows[2]);
            std::getline(stream, rows[3]);
            std::getline(stream, rows[4]);
            std::getline(stream, rows[5]);
            
            std::memset(room.data, Empty, sizeof(room));
            FillRow(rows[0], 0, room);
            FillRow(rows[1], 1, room);
            FillRow(rows[2], 2, room);
            FillRow(rows[3], 3, room);
            FillRow(rows[4], 4, room);
            FillRow(rows[5], 5, room);

            return true;
        }
    }

    return false;
}

void TRoomEntity::FillRow(const std::string& data, std::size_t row, TRoom& room) {
    for (std::size_t i = 0; i < 10 || i < data.size(); i++) {
        switch (data[i]) {
        case 'g': room.data[row * 10 + i] = Dirt; break;
        case '1': case '2':
        case '3': room.data[row * 10 + i] = LowDirt; break;
        case '4': case '5':
        case '6': room.data[row * 10 + i] = MidDirt; break;
        case '7': case '8':
        case '9': room.data[row * 10 + i] = HighDirt; break;
        case 'G': room.data[row * 10 + i] = Grass; break;
        case 'l': room.data[row * 10 + i] = Ladder; break;
        case 'd': room.data[row * 10 + i] = Mine; break;
        case 'D': room.data[row * 10 + i] = Spike; break;
        case 'p': room.data[row * 10 + i] = Passage; break;
        case 'P': room.data[row * 10 + i] = Plank; break;
        case 'c': room.data[row * 10 + i] = Curse; break;
        case 'b': room.data[row * 10 + i] = Baddy; break;
        case 'u': room.data[row * 10 + i] = Utility; break;
        default: break;
        }
    }
}

bool TRoomEntity::GoLeft(int* maze, NGame::Vec2i& position) {
    if (position.X > 0 && maze[ROOM_WIDTH * position.Y + position.X - 1] != 0) {
        return true;
    }

    if (position.X <= 0) {
        maze[ROOM_WIDTH * position.Y + position.X] |= Down;
        if (!GoDown(maze, position)) {
            return false;
        }
        return GoRight(maze, position);
    } else {
        position.X--;
        if (maze[ROOM_WIDTH * position.Y + position.X]) {
            position.X++;
        } else {
            maze[ROOM_WIDTH * position.Y + position.X] = LeftRight;
        }
    }

    return true;
}

bool TRoomEntity::GoRight(int* maze, NGame::Vec2i& position) {
    if (position.X + 1 >= ROOM_WIDTH) {
        maze[ROOM_WIDTH * position.Y + position.X] |= Down;
        if (!GoDown(maze, position)) {
            return false;
        }
        return GoLeft(maze, position);
    } else {
        position.X++;
        if (maze[ROOM_WIDTH * position.Y + position.X]) {
            position.X--;
        } else {
            maze[ROOM_WIDTH * position.Y + position.X] = LeftRight;
        }
    }

    return true;
}

bool TRoomEntity::GoDown(int* maze, NGame::Vec2i& position) {
    if (position.Y + 1 >= ROOM_HEIGHT) {
        maze[ROOM_WIDTH * position.Y + position.X] |= Exit;
        return false;
    } else {
        maze[ROOM_WIDTH * position.Y + position.X] |= Down;
        position.Y++;
        maze[ROOM_WIDTH * position.Y + position.X] = LeftRightUp;
    }

    return true;
}

void TRoomEntity::GenerateLayout() {
    int maze[ROOM_HEIGHT * ROOM_WIDTH];

    NGame::Vec2i position = {0, 0};
    std::memset(maze, None, sizeof(maze));

    position.X = rand() % ROOM_WIDTH;
    maze[ROOM_WIDTH * position.Y  + position.X] = LeftRight | Start;

    // Generate rooms
    while (true) {
        int choice = rand() % 100;

        if (choice < 33) {
            if (!GoLeft(maze, position)) {
                break;
            }
        } else if (choice < 66) {
            if (!GoRight(maze, position)) {
                break;
            }
        } else {
            if (!GoDown(maze, position)) {
                break;
            }
        }
    }

    for (int i = 0; i < ROOM_HEIGHT; i++) {    
        for (int j = 0; j < ROOM_WIDTH; j++) {
            auto item = maze[i * ROOM_WIDTH + j];
            GenerateRoom(maze, NGame::Vec2i(j, i));
        }
    }
}

void TRoomEntity::GenerateRoom(int* maze, const NGame::Vec2i& position) {

    auto roomType = maze[ROOM_WIDTH * position.Y + position.X];
    TRoom sourceRoom;
    int randomRoomChance = rand() % 100;

    SelectRoom(roomType & LeftRightUpDown, sourceRoom);
    for (int i = 0; i < 6; i++) {    
        for (int j = 0; j < 10; j++) {
            auto tile = sourceRoom.data[i * 10 + j];
            NGame::Vec2i entityPosition = {j * 16, i * 16};
            entityPosition += NGame::Vec2i(position.X * 160, position.Y * 96);
            std::shared_ptr<NGame::TEntity> entity;

            switch (tile) {
            default:
            case Empty:
                break;

            case LowDirt:
                if (rand() % 100 < 25) {
                    entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("DirtEntity");
                }
                break;
            
            case MidDirt:
                if (rand() % 100 < 50) {
                    entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("DirtEntity");
                }
                break;

            case HighDirt:
                if (rand() % 100 < 75) {
                    entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("DirtEntity");
                }
                break;

            case Dirt:
                entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("DirtEntity");
                break;

            case Grass:
                entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("GrassEntity");
                break;

            case Ladder:
                entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("LadderEntity");
                break;
            
            case Plank:
                entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("PlankEntity");
                break;

            case Mine:
                if (NGame::TApp::Instance()->State().Variable("MoreMines").Bool()) {
                    if (rand() % 100 < 50) {
                        entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("MineEntity");
                    }
                } else {
                    if (rand() % 100 < 25) {
                        entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("MineEntity");
                    }
                }
                break;

            case Passage:
                if (roomType & (Start | Exit)) {
                    entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("EntranceEntity");
                }
                break;

            case Curse:
                if (rand() % 100 < 25) {
                    entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("CurseEntity");
                }
                break;

            case Spike:
                if (rand() % 100 < 75) {
                    entity = NGame::TApp::Instance()->EntityManager().MakeEntityByName("SpikeEntity");
                }
                break;
            }

            if (!entity) {
                continue;
            }

            entity->SetPosition(entityPosition);
            NGame::TApp::Instance()->EntityManager().UpdateCollision(entity);
        }
    }
}

void TRoomEntity::SelectRoom(int type, TRoom& room) {
    std::vector<TRoom> selection;

    if (type == LeftRight) {
        selection.insert(selection.end(), LRRooms_.begin(), LRRooms_.end());
        selection.insert(selection.end(), LRDRooms_.begin(), LRDRooms_.end());
        selection.insert(selection.end(), LRURooms_.begin(), LRURooms_.end());
        selection.insert(selection.end(), LRUDRooms_.begin(), LRUDRooms_.end());
        room = selection[rand() % selection.size()];
    } else if (type == LeftRightUp) {
        selection.insert(selection.end(), LRURooms_.begin(), LRURooms_.end());
        selection.insert(selection.end(), LRUDRooms_.begin(), LRUDRooms_.end());
        room = selection[rand() % selection.size()];
    } else if (type == LeftRightDown) {
        selection.insert(selection.end(), LRDRooms_.begin(), LRDRooms_.end());
        selection.insert(selection.end(), LRUDRooms_.begin(), LRUDRooms_.end());
        room = selection[rand() % selection.size()];
    } else if (type == LeftRightUpDown) {
        room = LRUDRooms_[rand() % LRUDRooms_.size()];
    } else {
        selection.insert(selection.end(), AnyRooms_.begin(), AnyRooms_.end());
        selection.insert(selection.end(), LRRooms_.begin(), LRRooms_.end());
        selection.insert(selection.end(), LRDRooms_.begin(), LRDRooms_.end());
        selection.insert(selection.end(), LRURooms_.begin(), LRURooms_.end());
        selection.insert(selection.end(), LRUDRooms_.begin(), LRUDRooms_.end());
        room = selection[rand() % selection.size()];
    }
}


THero::THero(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    SetSize(NGame::Vec2i(4, 12));
    SetPosition(NGame::Vec2i(0, 0));
    SetCollisionGroup(HERO_GROUP);
    NGame::TApp::Instance()->RenderManager().EnableLight(false);
    NGame::TApp::Instance()->RenderManager().SetDefaultLightColor(255, 0, 0);

    Alarm_.Set(0, 100);
}

void THero::Input(SDL_Event* event) {
    switch (event->type) {
    case SDL_KEYDOWN:
        if (event->key.keysym.sym == SDLK_UP || event->key.keysym.sym == 'w') {
            KeysHeld_[static_cast<int>(EKeys::Up)] = true;
            KeysPressed_[static_cast<int>(EKeys::Up)] = true;
        }
        if (event->key.keysym.sym == SDLK_DOWN || event->key.keysym.sym == 's') {
            KeysHeld_[static_cast<int>(EKeys::Down)] = true;
            KeysPressed_[static_cast<int>(EKeys::Down)] = true;
        }
        if (event->key.keysym.sym == SDLK_LEFT || event->key.keysym.sym == 'a') {
            KeysHeld_[static_cast<int>(EKeys::Left)] = true;
            KeysPressed_[static_cast<int>(EKeys::Left)] = true;
        }
        if (event->key.keysym.sym == SDLK_RIGHT || event->key.keysym.sym == 'd') {
            KeysHeld_[static_cast<int>(EKeys::Right)] = true;
            KeysPressed_[static_cast<int>(EKeys::Right)] = true;
        }
        if (event->key.keysym.sym == 'z') {
            KeysHeld_[static_cast<int>(EKeys::Z)] = true;
            KeysPressed_[static_cast<int>(EKeys::Z)] = true;
        }
        if (event->key.keysym.sym == 'x') {
            KeysHeld_[static_cast<int>(EKeys::X)] = true;
            KeysPressed_[static_cast<int>(EKeys::X)] = true;
        }
        if (event->key.keysym.sym == 'c') {
            KeysHeld_[static_cast<int>(EKeys::C)] = true;
            KeysPressed_[static_cast<int>(EKeys::C)] = true;
        }
        if (event->key.keysym.sym == SDLK_LSHIFT) {
            KeysHeld_[static_cast<int>(EKeys::Shift)] = true;
            KeysPressed_[static_cast<int>(EKeys::Shift)] = true;
        }
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            KeysHeld_[static_cast<int>(EKeys::Escape)] = true;
            KeysPressed_[static_cast<int>(EKeys::Escape)] = true;
        }
        break;
    
    case SDL_KEYUP:
        if (event->key.keysym.sym == SDLK_UP || event->key.keysym.sym == 'w') {
            KeysHeld_[static_cast<int>(EKeys::Up)] = false;
        }
        if (event->key.keysym.sym == SDLK_DOWN || event->key.keysym.sym == 's') {
            KeysHeld_[static_cast<int>(EKeys::Down)] = false;
        }
        if (event->key.keysym.sym == SDLK_LEFT || event->key.keysym.sym == 'a') {
            KeysHeld_[static_cast<int>(EKeys::Left)] = false;
        }
        if (event->key.keysym.sym == SDLK_RIGHT || event->key.keysym.sym == 'd') {
            KeysHeld_[static_cast<int>(EKeys::Right)] = false;
        }
        if (event->key.keysym.sym == 'z') {
            KeysHeld_[static_cast<int>(EKeys::Z)] = false;
        }
        if (event->key.keysym.sym == 'x') {
            KeysHeld_[static_cast<int>(EKeys::X)] = false;
        }
        if (event->key.keysym.sym == 'c') {
            KeysHeld_[static_cast<int>(EKeys::C)] = false;
        }
        if (event->key.keysym.sym == SDLK_LSHIFT) {
            KeysHeld_[static_cast<int>(EKeys::Shift)] = false;
        }
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            KeysHeld_[static_cast<int>(EKeys::Escape)] = false;
        }
        break;
    }
}



void THero::Update(std::uint32_t delta) {
    auto app = NGame::TApp::Instance();
    auto& entityManager = app->EntityManager();

    for (bool repeatState = true; repeatState; ) {
        repeatState = false;

        switch (State_) {
        case EState::Normal:
            Speed_.X = 0;
            Speed_.Y += MovementPerTick(delta, Gravity_);

            if (KeysHeld_[static_cast<int>(EKeys::Right)]) {
                Speed_.X = 100;
            }
            if (KeysHeld_[static_cast<int>(EKeys::Left)]) {
                Speed_.X = -100;
            }
            if (KeysPressed_[static_cast<int>(EKeys::X)]) {
                auto entity = entityManager.MakeEntityByName("FloatingTextEntity");
                auto targetEntity = dynamic_cast<TFloatingTextEntity*>(entity.get());
                targetEntity->SetText("You died, LMAO");
                targetEntity->SetColor(NGame::TFontManager::Red);
                targetEntity->SetPosition(Position());
            }

            if (!entityManager.IsPlaceEmpty(Position(), Size(), LADDER_GROUP)) {
                if (KeysHeld_[static_cast<int>(EKeys::Up)] || KeysHeld_[static_cast<int>(EKeys::Down)]) {
                    State_ = EState::Climb;
                    continue;
                }
            }
            if (!entityManager.IsPlaceEmpty(Position() + NGame::Vec2i(-2, Size().Y), NGame::Vec2i(Size().X + 4, 1), TERRAIN_GROUP, Id())) {
                if (KeysHeld_[static_cast<int>(EKeys::Down)]) {
                    if (KeysPressed_[static_cast<int>(EKeys::Z)]) {
                        IgnorePlanks_ = true;
                        Speed_.Y = 80;
                        State_ = EState::Fall;
                        continue;
                    }
                } else {
                    if (KeysPressed_[static_cast<int>(EKeys::Z)]) {
                        Speed_.Y = -std::sqrt(2 * Gravity_ * (16 + (16 - Size().Y)));
                        State_ = EState::Jump;
                        continue;
                    }
                }
            }
            if (MovementPerTick(delta, Speed_.Y) >= 1.0) {
                State_ = EState::Fall;
                continue;
            }

            LastStablePosition_ = Position();

            break;

        case EState::Jump:
            Speed_.X = 0;
            Speed_.Y += MovementPerTick(delta, Gravity_);

            if (!entityManager.IsPlaceEmpty(Position(), Size(), LADDER_GROUP)) {
                if (KeysHeld_[static_cast<int>(EKeys::Up)] || KeysHeld_[static_cast<int>(EKeys::Down)]) {
                    State_ = EState::Climb;
                    continue;
                }
            }
            if (KeysHeld_[static_cast<int>(EKeys::Right)]) {
                Speed_.X = 100;
            }
            if (KeysHeld_[static_cast<int>(EKeys::Left)]) {
                Speed_.X = -100;
            }
            if (Speed_.Y > 0) {
                State_ = EState::Fall;
                continue;
            }
            break;

        case EState::Fall:
            Speed_.X = 0;
            Speed_.Y += MovementPerTick(delta, Gravity_);

            if (!entityManager.IsPlaceEmpty(Position(), Size(), LADDER_GROUP)) {
                if (KeysHeld_[static_cast<int>(EKeys::Up)] || KeysHeld_[static_cast<int>(EKeys::Down)]) {
                    State_ = EState::Climb;
                    continue;
                }
            }
            if (KeysHeld_[static_cast<int>(EKeys::Right)]) {
                Speed_.X = 100;
                auto collisonIds = entityManager.CollisionList(Position() + NGame::Vec2i(Size().X, 0), NGame::Vec2i(Size().X, 2), TERRAIN_GROUP, Id());
                auto containsWall = std::any_of(collisonIds.begin(), collisonIds.end(), [&](TEntity::TId id) {
                    auto other = entityManager.Entity(id);
                    if (dynamic_cast<TPlankEntity*>(other.get())) {
                        return false;
                    }
                    return true;
                });

                if (containsWall) {
                    if (entityManager.IsPlaceEmpty(Position() + NGame::Vec2i(0, -Size().Y), NGame::Vec2i(Size().X * 2, Size().Y), TERRAIN_GROUP, Id())) {
                        State_ = EState::Hold;
                        continue;
                    }
                }
            }
            if (KeysHeld_[static_cast<int>(EKeys::Left)]) {
                Speed_.X = -100;
                auto collisonIds = entityManager.CollisionList(Position() - NGame::Vec2i(Size().X, 0), NGame::Vec2i(Size().X, 2), TERRAIN_GROUP, Id());
                auto containsWall = std::any_of(collisonIds.begin(), collisonIds.end(), [&](TEntity::TId id) {
                    auto other = entityManager.Entity(id);
                    if (dynamic_cast<TPlankEntity*>(other.get())) {
                        return false;
                    }
                    return true;
                });

                if (containsWall) {
                    if (entityManager.IsPlaceEmpty(Position() - NGame::Vec2i(Size().X, Size().Y), NGame::Vec2i(Size().X * 2, Size().Y), TERRAIN_GROUP, Id())) {
                        State_ = EState::Hold;
                        continue;
                    }
                }
            }
            if (!entityManager.IsPlaceEmpty(Position() + NGame::Vec2i(0, Size().Y), NGame::Vec2i(Size().X, 1), TERRAIN_GROUP, Id())) {
                State_ = EState::Normal;
                continue;
            }
            break;

        case EState::Climb:
            if (entityManager.IsPlaceEmpty(Position(), Size(), LADDER_GROUP)) {
                State_ = EState::Normal;
                repeatState = true;
                continue;
            }
            
            Speed_ = {0, 0};
            if (KeysHeld_[static_cast<int>(EKeys::Right)]) {
                Speed_.X = 100;
            }
            if (KeysHeld_[static_cast<int>(EKeys::Left)]) {
                Speed_.X = -100;
            }
            if (KeysHeld_[static_cast<int>(EKeys::Up)]) {
                Speed_.Y = -100;
            }
            if (KeysHeld_[static_cast<int>(EKeys::Down)]) {
                Speed_.Y = 100;
            }
            LastStablePosition_ = Position();
            break;

        case EState::Hold:
            Speed_ = 0;
            if (KeysPressed_[static_cast<int>(EKeys::Down)]) {
                State_ = EState::Fall;
                continue;
            }
            if (KeysPressed_[static_cast<int>(EKeys::Z)]) {
                Speed_.Y = -std::sqrt(2 * Gravity_ * (16 + (16 - Size().Y)));
                State_ = EState::Jump;
                continue;
            }
            LastStablePosition_ = Position();
            break;

        case EState::Dead:
            Speed_.Y += MovementPerTick(delta, Gravity_);
            break;
        }   
    }

    // Explictly check damage sources and flick the body around the screen
    {
        auto entityList = entityManager.CollisionList(Position(), Size(), DAMAGE_GROUP, Id());
        auto outCenter = Position() + Size() / 2;

        for (auto& otherId : entityList) {
            auto other = entityManager.Entity(otherId);
            
            if (dynamic_cast<TExplosionEntity*>(other.get())) {
                State_ = EState::Dead;

                auto explosionCenter = other->Position() + other->Size() / 2;
                auto diff = outCenter - explosionCenter;
                if (diff.X) {
                    Speed_.X += 500.0 / diff.X;
                }
                if (diff.Y) {
                    Speed_.Y += 500.0 / diff.Y;
                }
            } else if (dynamic_cast<TSpikeEntity*>(other.get())) {
                if (MovementPerTick(delta, Speed_.Y) < 1.0) {
                    continue;
                };

                State_ = EState::Dead;
                auto spike = dynamic_cast<TSpikeEntity*>(other.get());
                spike->SetBloody(true);
                
                Speed_.X = 0;
                if (Speed_.Y > 5) {
                    Speed_.Y = 5;
                }
            }
        }
    }
    
    // Check if we need to flip sprite
    if (Speed_.X < 0) {
        FaceLeft_ = 1;
    } else if (Speed_.X > 0) {
        FaceLeft_ = 0;
    }

    // Perform movement and collision
    NGame::Vec2f resultSpeed = MovementPerTick(delta, Speed_);
    auto moveResult = MoveWithCondition(resultSpeed, Fraction_, [&](const NGame::Vec2i& position) {
        auto collisionList = entityManager.CollisionList(position, Size(), TERRAIN_GROUP, Id());
        if (collisionList.empty()) {
            return true;
        }

        // Check collision for planks
        for (auto& otherId : collisionList) {
            auto other = entityManager.Entity(otherId);
            if (dynamic_cast<TPlankEntity*>(other.get())) {
                // Are we ignoring planks, because we are dropping down?
                if (IgnorePlanks_) {
                    continue;
                }

                // Ignore planks if we moving up
                if (Speed_.Y < 0) {
                    continue;
                }

                // Ignore planks, that are already inside of us
                if (NGame::RectOverlaps(Position(), Size(), other->Position(), other->Size())) {
                    continue;
                }
                
                // If all fails - we should be on top of the plank
                return false;
            } else {
                return false;
            }
        }
        
        return true;
    });

    Fraction_ = moveResult.first;
    if (State_ != EState::Dead) {
        if (!moveResult.second.first) {
            Speed_.X = 0;
        }
        if (!moveResult.second.second) {
            // Fall damage
            if (app->State().Variable("FallDamage").Bool() && (Position().Y - LastStablePosition_.Y > 40)) {
                State_ = EState::Dead;
            }

            Speed_.Y = 0;
        }
    } else {
        if (!moveResult.second.first) {
            Speed_.X = Speed_.X * -0.5f;
        }
        if (!moveResult.second.second) {
            Speed_.Y = Speed_.Y * -0.5f;
            Speed_.X = Speed_.X * 0.8f;
        }
    }
    
    // Set camera to track hero
    app->RenderManager().SetCamera(Position() - app->RenderManager().Size() / 2);

    // Reset pressed keys
    IgnorePlanks_ = false;
    std::memset(KeysPressed_, false, sizeof(KeysPressed_));
}

void THero::Draw() const {
    auto& renderManager = NGame::TApp::Instance()->RenderManager();
    auto& spriteManager = NGame::TApp::Instance()->SpriteManager();

    renderManager.SetLayer(NGame::TRenderManager::Foreground);
    auto hero = spriteManager.Get("Sprites/Hero.txt");
    int spriteIndex = 0;


    switch (State_) {
    case EState::Normal: spriteIndex = 0; break;
    case EState::Jump: spriteIndex = 2; break;
    case EState::Fall: spriteIndex = 0; break;
    case EState::Climb: spriteIndex = 5; break;
    case EState::Hold: spriteIndex = 1; break;
    case EState::Dead: spriteIndex = 4; break;
    }

    if (State_ == EState::Normal && Speed_.X != 0 && AlternateRun_) {
        spriteIndex = 3;
    }

    if (State_ != EState::Dead) {
        spriteManager.Draw(hero, spriteIndex, Position() - NGame::Vec2i((16 - Size().X) / 2, 16 - Size().Y), {(FaceLeft_?-1.0f:1.0f), 1.0f});
    } else {
        spriteManager.Draw(hero, spriteIndex, Position() - NGame::Vec2i((16 - Size().X) / 2, 16 - Size().Y - 3), {(FaceLeft_?-1.0f:1.0f), 1.0f});
    }
    
    renderManager.SetLayer(NGame::TRenderManager::Light);
    auto light = spriteManager.Get("Sprites/Light.txt");
    spriteManager.Draw(light, 0, Position() - (light->Frames[0].Size * 4 / 2), {4, 4});
}

void THero::Alarm(NGame::TAlarm::TId id) {
    if (id == 0) {
        AlternateRun_ = !AlternateRun_;
    }
}

TBackgroundTiler::TBackgroundTiler(NGame::TEntity::TId id)
    : NGame::TEntity(id) {
    Background_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Wall.txt");
}
void TBackgroundTiler::Draw() const {
    auto& renderManager = NGame::TApp::Instance()->RenderManager();
    auto& spriteManager = NGame::TApp::Instance()->SpriteManager();
    
    auto cameraPosition = renderManager.Camera();
    auto tileOffset = cameraPosition;
    tileOffset.X %= 16;
    tileOffset.Y %= 16;


    renderManager.SetLayer(NGame::TRenderManager::Tile);
    for (int i = -1; i <= 320 / 16; ++i) {
        for (int j = -1; j <= 240 / 16; ++j) {
            spriteManager.Draw(Background_, 0, NGame::Vec2i(i * 16, j * 16) + cameraPosition - tileOffset);
        }
    }
}

TRubbleEntity::TRubbleEntity(NGame::TEntity::TId id) 
    : NGame::TEntity(id) {
    SetSize(8);
    Sprite_ = NGame::TApp::Instance()->SpriteManager().Get("Sprites/Stone.txt");
    Alarm_.Set(0, 5000);
}

void TRubbleEntity::Update(std::uint32_t delta) {
    auto& entityManager = NGame::TApp::Instance()->EntityManager();

    Speed_.Y += MovementPerTick(delta, Gravity_);
    NGame::Vec2f resultSpeed = MovementPerTick(delta, Speed_);
    auto moveResult = MoveWithCondition(resultSpeed, Fraction_, [&](const NGame::Vec2i& position) {
        auto collisionList = entityManager.IsPlaceEmpty(position, Size(), TERRAIN_GROUP, Id());
        if (collisionList) {
            return true;
        }
        return false;
    });

    if (!moveResult.second.first) {
        Speed_.X = Speed_.X * -0.25f;
    }
    if (!moveResult.second.second) {
        Speed_.Y = Speed_.Y * -0.25f;
        Speed_.X = Speed_.X * 0.25;
    }
}

void TRubbleEntity::Draw() const {
    auto& renderManager = NGame::TApp::Instance()->RenderManager();
    auto& spriteManager = NGame::TApp::Instance()->SpriteManager();

    renderManager.SetLayer(NGame::TRenderManager::Foreground);
    spriteManager.Draw(Sprite_, 0, Position());
}

void TRubbleEntity::Alarm(NGame::TAlarm::TId id) {
    if (id == 0) {
        Remove();
    }
}

const NGame::Vec2f& TRubbleEntity::Speed() const {
    return Speed_;
}

void TRubbleEntity::SetSpeed(const NGame::Vec2f& speed) {
    Speed_ = speed;
}