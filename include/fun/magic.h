#ifndef SVKFW_MAGIC_H
#define SVKFW_MAGIC_H

#include <vector>

#include "math/vectors.h"


namespace Fun {
//  == === ==== >  Magic elements

    // Имеют усиливающую и ослабляющую заклинание синергии. Есть некоторая базовая синергия, которая включена по умолчанию.
    // Усиливающая синергия делает заклинание дороже. Отключить ослабляющую синергию стоит дороже.
    enum MagicElementBit : uint32_t {
        MAGIC_ELEMENT_NONE    = 0u,

        /*
            Magical force
            Cost: area and value.
            Notes:
            . Push, pull, levitate (nullify gravity or apply counter-force, which is two different things);
            . Attack is also possible;
            . Massage/Scratch/etc;
            . Essential part of any barrier spell;

            . Note: barrier caster can use other elements to "encrypt" it;
                    you can "decrypt" it by applying some magic and "reading" response, but intense magic scan
                    can be used to change barrier structure on-the-fly (keeping elements)
        */
        MAGIC_ELEMENT_FORCE   = 1u <<  0,

        /*
            Magical matter in the gas state
            Cost: speed and mass.
            Notes:
            . Air by default;
        */
        MAGIC_ELEMENT_GAS     = 1u <<  1,

        /*
            Magical matter in the solid state
            Cost: speed and mass.
            Notes:
            . Earth (element) by default;
        */
        MAGIC_ELEMENT_SOLID   = 1u <<  2,

        /*
            Magical matter in the liquid state
            Cost: speed and mass.
            Notes:
            . Water by default;
        */
        MAGIC_ELEMENT_LIQUID  = 1u <<  3,

        /*
            Magical matter in the plasma state
            Cost: volume and temperature.
            Notes:
            . Can be obtained by combining 'GAS' and 'ELECTRO'
        */
        MAGIC_ELEMENT_PLASMA  = 1u <<  4,

        /*
            Magical heat or cold
            Cost: volume and temperature difference.
        */
        MAGIC_ELEMENT_HEAT    = 1u <<  5,

        /*
            Magical fire
            Cost: volume and temperature.
            Notes:
            . Full "FIRE" experience includes: "SOUND", "MIND", "AIR", "HEAT";
            . Not hot by itself, just an effect;
        */
        MAGIC_ELEMENT_FIRE    = 1u <<  6,

        /*
            Magical electricity
            Cost: electricity power.
            Notes:
            . Full "ELECTRO" experience includes: "SOUND", "MIND", "HEAT";
            . Default interpolation speed is 1/3 of speed of light -- interpolation costs nothing below this speed;
        */
        MAGIC_ELEMENT_ELECTRO = 1u <<  7,

        /*
            boom
            Cost: explosion power.
            Notes:
            . Full "EXPLOSION" experience includes: "SOUND", "MIND", "HEAT", "FIRE", "FORCE";
            . "MIND" is not necessary to affect entities with "mind" (only because they can be affected indirectly, by the aftermath of the explosion);
        */
        MAGIC_ELEMENT_EXPLODE = 1u <<  8,

        /*
            Magic to (partially) restore an object to its "normal" form
            Cost: difference in states (and stats) before/after.
            Notes:
            . Use with "MIND" to apply to entities with "mind" (otherwise it can "repair" inanimate objects);
            . The restored state can be some previous state or a combination of previous states;
            . Combine with "TFORM" to achieve features they never had (to restore eyesight for someone who's blind since birth);
            . To grow a third arm, use "TFORM" with "MIND", preferrably with "HEAL", otherwise it will be painful;
        */
        MAGIC_ELEMENT_HEAL    = 1u <<  9,

        /*
            Manipulations involving entities with "mind" - people, creatures, intelligent machines
            Cost: base entity stats and difference in stats after applying the spell.
            Notes:
            . On by default;
            . Turning it off descreases the cost;
        */
        MAGIC_ELEMENT_MIND    = 1u << 10,

        /*
            Transform one object into another
            Cost: base_value + max(0, difference in strength).
            Notes:
            . Use "MIND" when transforming (-/from/into) entity with "mind" (like growing a third arm, werewolf transformation, etc.),
              preferrably together with "HEAL";
            . When transforming inanimate object into living object, System gives it mind
        */
        MAGIC_ELEMENT_TFORM   = 1u << 11,

        /*
            Magical sound (and Magical mechanical waves)
            Cost: amplitude and scale.
            Notes:
            . Default interpolation speed is speed of sound -- interpolation costs nothing below this speed;
            . On by default;
            . Makes the spell audible;
            . To silence the spell, turn this element off (increases the cost);
            . To start an earthquake, use this;
        */
        MAGIC_ELEMENT_SOUND   = 1u << 12,

        /*
            Magcal light (and Magical EM waves)
            Cost: intensity.
            Notes:
            . Default interpolation speed is speed of light -- interpolation costs nothing;
            . Includes radio waves (can be used for communication);
            . Includes microwaves, X-rays, etc.;
        */
        MAGIC_ELEMENT_LIGHT   = 1u << 13,

        /*
            Magical void
            Cost: volume and absorbing rate.
            Notes:
            . Default interpolation speed is speed of light -- interpolation costs nothing;
            . Absorbs light, all sorts of energy and even entities;
            . Note: invisibility requires both 'LIGHT' + 'VOID'
        */
        MAGIC_ELEMENT_VOID    = 1u << 14,

        MAGIC_ELEMENT_ALL = 0x7fff,
        MAGIC_ELEMENT_MAX = 0xffffffff,
    }; // MagicElementBit END
    typedef uint32_t MagicElements;


//  == === ==== >  Magic synergy

    enum MagicSynergyEnm : uint32_t {
        MAGIC_SYNERGY_NONE,
        MAGIC_SYNERGY_MAX = 0xffffffff,
    }; // MagicSynergyEnm END
    typedef uint32_t MagicSynergy;


//  == === ==== >  Magic step

    enum MagicStepShapeEnm : uint32_t {
        MAGIC_SHAPE_CIRCLE,
        MAGIC_SHAPE_POLYGON, 
        MAGIC_SHAPE_SPHERE,
        MAGIC_SHAPE_POLYHEDRON,
        MAGIC_SHAPE_POINT,
    }; // MagicStepShapeEnm END
    typedef uint32_t MagicStepShape;

    struct MagicStep {
        Simple::vec4f point3_radius1; // relative to the starting point of cast
        float wait_time;
        MagicStepShape shape;
        uint32_t interp_coefs; // 4 uint8_t values: (x1, y1), (x2, y2)
        uint32_t flag_laststep; // padding + 1u if first step | 2u if last step
    }; // MagicStep END

    // Место              старта каста - непосредственно от кастующего (рука, палочка, жезл и т.д., предметы могут удешевлять) или на расстоянии (дороже)
    // Форма              старта каста - точка, круг и т.д. (чем больше радиус, тем дороже)
    // Эффект          на старте каста - эффект заклинания в этой точке
    // Время задержки  на старте каста - чем дольше, тем ДЕШЕВЛЕ заклинание
    // Интерполяция    от старта каста - интерполяция (линейно/полиномиально - неважно, влияет только время перехода: чем быстрее, тем дороже)
    // Переход эффекта от старта каста - none/move/copy - (copy - позволяет сосредоточить магию между этой формой и следующей, например для создания барьера)
    // Эффект перехода на старте каста - эффект заклинания во время перехода (держится, если тип перехода copy)

    // Место              промежуточной точки каста 1 - строго по прямой, по фиксированной кривой, наводка на объект (от дешевого к дорогому, также чем дальше, тем дороже)
    // Форма              промежуточной точки каста 1 - точка, круг и т.д. (чем больше радиус, тем дороже)
    // Эффект             промежуточной точки каста 1 - эффект заклинания в этой точке
    // Время задержки  на промежуточной точке каста 1 - чем дольше, тем ДОРОЖЕ заклинание
    // Интерполяция    от промежуточной точки каста 1 - интерполяция (линейно/полиномиально - неважно, влияет только время перехода: чем быстрее, тем дороже)
    // Тип перехода    от промежуточной точки каста 1 - move/copy - (copy - позволяет сосредоточить магию между этой формой и следующей, например для создания барьера. Copy существенно дороже, чем move)
    // Эффект перехода на промежуточной точке каста 1 - эффект заклинания во время перехода (держится, если тип перехода copy)

    // Место              точки окончания каста - строго по прямой, по фиксированной кривой, наводка на объект (от дешевого к дорогому, также чем дальше, тем дороже)
    // Форма              точки окончания каста - точка, круг и т.д. (чем больше радиус, тем дороже)
    // Эффект             точки окончания каста - эффект заклинания в этой точке
    // Время задержки  на точке окончания каста - чем дольше, тем ДОРОЖЕ заклинание
    // Интерполяция    от точки окончания каста - unused
    // Тип перехода    от точки окончания каста - unused
    // Эффект перехода на точке окончания каста - unused


//  == === ==== >  Magic point

    // TODO: Probably implement as small (64x64?) binary masks representing the shape of the spell.
    //       When the spell applied to an object, use it as a map.

    struct MagicPoint {
        // TODO: decide on shape representation. Don't forget 'scale' parameter.
        MagicPoint() {}
        MagicPoint(const MagicPoint &) = default;
    }; // MagicPoint END


//  == === ==== >  Magic spell

    struct MagicSpell {
        MagicElements elements = MAGIC_ELEMENT_NONE;
        MagicSynergy   synergy = MAGIC_SYNERGY_NONE;
        uint32_t   step_offset = UINT32_MAX; // MagicStep offset in MagicSpellbook array
        float        raw_power = 0.f; // spell's life = power; decreases when spell hits some object; power at the last step is the strength of the spell
    }; // MagicSpell END


    struct MagicSpellbook {
        std::vector<MagicStep>  magic_steps;
        std::vector<MagicSpell> magic_spells;
    }; // MagicSpellbook END
}; // Fun END

#endif