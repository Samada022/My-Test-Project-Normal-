#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

// Скорость пули (юниты GD в секунду) и время жизни (сек)
constexpr float BULLET_SPEED = 900.f;
constexpr float BULLET_LIFETIME = 2.0f;

struct Bullet {
    Ref<CCSprite> sprite;
    float life = 0.f;
};

class $modify(ShootPlayLayer, PlayLayer) {
    struct Fields {
        std::vector<Bullet> bullets;
    };

    void shoot() {
        if (!m_player1 || m_player1->m_isDead) return;

        auto sprite = CCSprite::createWithSpriteFrameName("square_01_001.png");
        sprite->setScale(0.25f);
        sprite->setColor({255, 80, 80});
        sprite->setPosition(m_player1->getPosition() + ccp(20.f, 0.f));
        sprite->setZOrder(1000);
        m_objectLayer->addChild(sprite);

        m_fields->bullets.push_back({sprite, 0.f});
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        auto& bullets = m_fields->bullets;
        for (auto it = bullets.begin(); it != bullets.end();) {
            auto& b = *it;
            b.life += dt;
            b.sprite->setPositionX(b.sprite->getPositionX() + BULLET_SPEED * dt);

            bool remove = b.life > BULLET_LIFETIME;

            if (!remove) {
                auto pos = b.sprite->getPosition();
                CCRect bulletRect(pos.x - 6.f, pos.y - 6.f, 12.f, 12.f);

                for (auto obj : CCArrayExt<GameObject*>(m_objects)) {
                    if (!obj || !obj->isVisible()) continue;

                    auto type = obj->m_objectType;
                    if (type != GameObjectType::Hazard &&
                        type != GameObjectType::Solid &&
                        type != GameObjectType::Slope &&
                        type != GameObjectType::Breakable) continue;

                    if (bulletRect.intersectsRect(obj->getObjectRect())) {
                        obj->deactivateObject(true); // убираем объект
                        remove = true;
                        break;
                    }
                }
            }

            if (remove) {
                b.sprite->removeFromParent();
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }
    }

    void resetLevel() {
        for (auto& b : m_fields->bullets) b.sprite->removeFromParent();
        m_fields->bullets.clear();
        PlayLayer::resetLevel();
    }
};

class $modify(ShootKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double timestamp) {
        if (down && !repeat && key == KEY_G) {
            if (auto pl = static_cast<ShootPlayLayer*>(PlayLayer::get())) {
                pl->shoot();
            }
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, timestamp);
    }
};
