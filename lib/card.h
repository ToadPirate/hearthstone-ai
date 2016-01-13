#ifndef _CARD_H
#define _CARD_H

struct Card
{
	int id;

	enum Type {
		MINION, SPELL, WEAPON, SECRET
	} type;

	struct Minion {
		int cost;
		int attack;
		int hp_max;
		int hp;
	};

	struct Spell {
		// TODO
	};

	struct Weapon {
		int cost;
		int attack;
		int duration;
	};

	struct Secret {
		// TODO
	};

	union {
		Minion minion;
		Spell spell;
		Weapon weapon;
		Secret secret;
	};
};

#endif
