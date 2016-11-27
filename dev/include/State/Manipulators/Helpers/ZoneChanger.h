#pragma once

#include "State/Types.h"
#include "State/Cards/Card.h"
#include "State/State.h"
#include "State/board/Player.h"
#include "State/board/DefaultZonePosPolicy.h"

namespace FlowControl
{
	namespace Manipulators
	{
		namespace Helpers
		{
			template <state::CardType TargetCardType, state::CardZone TargetCardZone>
			class AddToPlayerDatStructure
			{
			public:
				static void Add(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					switch (card.GetZone())
					{
					case state::kCardZoneDeck:
						return AddToDeckZone(state, card_ref, card);
					case state::kCardZoneHand:
						return AddToHandZone(state, card_ref, card);
					case state::kCardZonePlay:
						return AddToPlayZone(state, card_ref, card);
					case state::kCardZoneGraveyard:
						return AddToGraveyardZone(state, card_ref, card);
					}
				}

			private:
				static void AddToDeckZone(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					state.board.Get(card.GetPlayerIdentifier())
						.deck_.GetLocationManipulator().Insert(state, card_ref);
				}

				static void AddToHandZone(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					state.board.Get(card.GetPlayerIdentifier())
						.hand_.GetLocationManipulator().Insert(state, card_ref);
				}

				static void AddToPlayZone(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					state::board::Player & player = state.board.Get(card.GetPlayerIdentifier());

					switch (TargetCardType)
					{
					case state::kCardTypeHero:
						if (player.hero_ref_.IsValid()) throw std::exception("hero should be removed first");
						player.hero_ref_ = card_ref;
						return;
					case state::kCardTypeMinion:
						return player.minions_.GetLocationManipulator().Insert(state, card_ref);
					case state::kCardTypeWeapon:
						return player.weapon_.Equip(card_ref);
					case state::kCardTypeSecret:
						return player.secrets_.Add(card.GetCardId(), card_ref);
					}
				}

				static void AddToGraveyardZone(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					state.board.Get(card.GetPlayerIdentifier())
						.graveyard_.GetLocationManipulator<TargetCardType>().Insert(state, card_ref);
				}
			};

			template <state::CardType RemovingCardType, state::CardZone RemovingCardZone>
			class RemoveFromPlayerDatStructure
			{
			public:
				static void Remove(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					switch (RemovingCardZone)
					{
					case state::kCardZoneDeck:
						return RemoveFromDeckZone(state, card_ref, card);
					case state::kCardZoneHand:
						return RemoveFromHandZone(state, card_ref, card);
					case state::kCardZonePlay:
						return RemoveFromPlayZone(state, card_ref, card);
					case state::kCardZoneGraveyard:
						return RemoveFromGraveyardZone(state, card_ref, card);
						break;
					}
				}

			private:
				static void RemoveFromDeckZone(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					state.board.Get(card.GetPlayerIdentifier())
						.deck_.GetLocationManipulator().Remove(state, card.GetZonePosition());
				}

				static void RemoveFromHandZone(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					state.board.Get(card.GetPlayerIdentifier())
						.hand_.GetLocationManipulator().Remove(state, card.GetZonePosition());
				}

				static void RemoveFromPlayZone(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					state::board::Player & player = state.board.Get(card.GetPlayerIdentifier());

					switch (RemovingCardType)
					{
					case state::kCardTypeMinion:
						return player.minions_.GetLocationManipulator().Remove(state, card.GetZonePosition());
					case state::kCardTypeWeapon:
						return player.weapon_.Destroy();
					case state::kCardTypeSecret:
						return player.secrets_.Remove(card.GetCardId());
					}
				}

				static void RemoveFromGraveyardZone(state::State & state, CardRef card_ref, state::Cards::Card & card)
				{
					state.board.Get(card.GetPlayerIdentifier())
						.graveyard_.GetLocationManipulator<RemovingCardType>().Remove(state, card.GetZonePosition());
				}
			};

			template <state::CardZone ChangingCardZone, state::CardType ChangingCardType>
			class ZoneChanger
			{
			public:
				ZoneChanger(state::State & state, CardRef card_ref, state::Cards::Card &card) : state_(state), card_ref_(card_ref), card_(card) {}

				template <state::CardZone ChangeToZone,
					typename std::enable_if_t<state::board::ForcelyUseDefaultZonePos<ChangeToZone, ChangingCardType>::value, nullptr_t> = nullptr>
					void ChangeTo(state::PlayerIdentifier player_identifier)
				{
					state::board::Player & player = state_.board.Get(card_.GetPlayerIdentifier());
					int new_pos = state::board::DefaultZonePosGetter<ChangeToZone, ChangingCardType>()(player);
					return ChangeToInternal<ChangeToZone>(player_identifier, new_pos);
				}

				template <state::CardZone ChangeToZone,
					typename std::enable_if_t<!state::board::ForcelyUseDefaultZonePos<ChangeToZone, ChangingCardType>::value, nullptr_t> = nullptr>
					void ChangeTo(state::PlayerIdentifier player_identifier, int pos)
				{
					return ChangeToInternal<ChangeToZone>(player_identifier, pos);
				}

				void Add()
				{
					AddToPlayerDatStructure<ChangingCardType, ChangingCardZone>::Add(state_, card_ref_, card_);
				}

			private:
				template <state::CardZone ChangeToZone>
				void ChangeToInternal(state::PlayerIdentifier player_identifier, int pos)
				{
					RemoveFromPlayerDatStructure<ChangingCardType, ChangingCardZone>::Remove(state_, card_ref_, card_);

					card_.SetLocation()
						.Player(player_identifier)
						.Zone(ChangeToZone)
						.Position(pos);

					AddToPlayerDatStructure<ChangingCardType, ChangeToZone>::Add(state_, card_ref_, card_);
				}

			private:
				state::State & state_;
				CardRef card_ref_;
				state::Cards::Card & card_;
			};

			template <state::CardType ChangingCardType>
			class ZoneChangerWithUnknownZone
			{
			public:
				ZoneChangerWithUnknownZone(state::State & state, CardRef card_ref, state::Cards::Card &card) : state_(state), card_ref_(card_ref), card_(card) {}

				template <state::CardZone ChangeToZone>
				void ChangeTo(state::PlayerIdentifier player_identifier)
				{
					switch (card_.GetZone())
					{
					case state::kCardZoneDeck:
						return ZoneChanger<state::kCardZoneDeck, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardZoneGraveyard:
						return ZoneChanger<state::kCardZoneGraveyard, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardZoneHand:
						return ZoneChanger<state::kCardZoneHand, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardZonePlay:
						return ZoneChanger<state::kCardZonePlay, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardZonePutASide:
						return ZoneChanger<state::kCardZonePutASide, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardZoneRemoved:
						return ZoneChanger<state::kCardZoneRemoved, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardZoneSecret:
						return ZoneChanger<state::kCardZoneSecret, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					default:
						throw std::exception("Unknown card zone");
					}
				}

				template <state::CardZone ChangeToZone>
				void ChangeTo(state::PlayerIdentifier player_identifier, int pos)
				{
					switch (card_.GetZone())
					{
					case state::kCardZoneDeck:
						return ZoneChanger<state::kCardZoneDeck, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardZoneGraveyard:
						return ZoneChanger<state::kCardZoneGraveyard, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardZoneHand:
						return ZoneChanger<state::kCardZoneHand, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardZonePlay:
						return ZoneChanger<state::kCardZonePlay, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardZonePutASide:
						return ZoneChanger<state::kCardZonePutASide, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardZoneRemoved:
						return ZoneChanger<state::kCardZoneRemoved, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardZoneSecret:
						return ZoneChanger<state::kCardZoneSecret, ChangingCardType>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					default:
						throw std::exception("Unknown card zone");
					}
				}

				void Add()
				{
					switch (card_.GetZone())
					{
					case state::kCardZoneDeck:
						return ZoneChanger<state::kCardZoneDeck, ChangingCardType>(state_, card_ref_, card_).Add();
					case state::kCardZoneGraveyard:
						return ZoneChanger<state::kCardZoneGraveyard, ChangingCardType>(state_, card_ref_, card_).Add();
					case state::kCardZoneHand:
						return ZoneChanger<state::kCardZoneHand, ChangingCardType>(state_, card_ref_, card_).Add();
					case state::kCardZonePlay:
						return ZoneChanger<state::kCardZonePlay, ChangingCardType>(state_, card_ref_, card_).Add();
					case state::kCardZonePutASide:
						return ZoneChanger<state::kCardZonePutASide, ChangingCardType>(state_, card_ref_, card_).Add();
					case state::kCardZoneRemoved:
						return ZoneChanger<state::kCardZoneRemoved, ChangingCardType>(state_, card_ref_, card_).Add();
					case state::kCardZoneSecret:
						return ZoneChanger<state::kCardZoneSecret, ChangingCardType>(state_, card_ref_, card_).Add();
					default:
						throw std::exception("Unknown card zone");
					}
				}

			private:
				state::State & state_;
				CardRef card_ref_;
				state::Cards::Card & card_;
			};

			class ZoneChangerWithUnknownZoneUnknownType
			{
			public:
				ZoneChangerWithUnknownZoneUnknownType(state::State& state, CardRef card_ref, state::Cards::Card &card) : state_(state), card_ref_(card_ref), card_(card) {}

				template <state::CardZone ChangeToZone>
				void ChangeTo(state::PlayerIdentifier player_identifier)
				{
					switch (card_.GetCardType())
					{
					case state::kCardTypeMinion:
						return ZoneChangerWithUnknownZone<state::kCardTypeMinion>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardTypeHeroPower:
						return ZoneChangerWithUnknownZone<state::kCardTypeHeroPower>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardTypeSecret:
						return ZoneChangerWithUnknownZone<state::kCardTypeSecret>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardTypeSpell:
						return ZoneChangerWithUnknownZone<state::kCardTypeSpell>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					case state::kCardTypeWeapon:
						return ZoneChangerWithUnknownZone<state::kCardTypeWeapon>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier);
					default:
						throw std::exception("unknown card type");
					}
				}

				template <state::CardZone ChangeToZone>
				void ChangeTo(state::PlayerIdentifier player_identifier, int pos)
				{
					switch (card_.GetCardType())
					{
					case state::kCardTypeHero:
						return ZoneChangerWithUnknownZone<state::kCardTypeHero>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardTypeMinion:
						return ZoneChangerWithUnknownZone<state::kCardTypeMinion>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardTypeHeroPower:
						return ZoneChangerWithUnknownZone<state::kCardTypeHeroPower>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardTypeSecret:
						return ZoneChangerWithUnknownZone<state::kCardTypeSecret>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardTypeSpell:
						return ZoneChangerWithUnknownZone<state::kCardTypeSpell>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					case state::kCardTypeWeapon:
						return ZoneChangerWithUnknownZone<state::kCardTypeWeapon>(state_, card_ref_, card_).ChangeTo<ChangeToZone>(player_identifier, pos);
					default:
						throw std::exception("unknown card type");
					}
				}

				void Add()
				{
					switch (card_.GetCardType())
					{
					case state::kCardTypeHero:
						return ZoneChangerWithUnknownZone<state::kCardTypeHero>(state_, card_ref_, card_).Add();
					case state::kCardTypeMinion:
						return ZoneChangerWithUnknownZone<state::kCardTypeMinion>(state_, card_ref_, card_).Add();
					case state::kCardTypeHeroPower:
						return ZoneChangerWithUnknownZone<state::kCardTypeHeroPower>(state_, card_ref_, card_).Add();
					case state::kCardTypeSecret:
						return ZoneChangerWithUnknownZone<state::kCardTypeSecret>(state_, card_ref_, card_).Add();
					case state::kCardTypeSpell:
						return ZoneChangerWithUnknownZone<state::kCardTypeSpell>(state_, card_ref_, card_).Add();
					case state::kCardTypeWeapon:
						return ZoneChangerWithUnknownZone<state::kCardTypeWeapon>(state_, card_ref_, card_).Add();
					default:
						throw std::exception("unknown card type");
					}
				}

			private:
				state::State & state_;
				CardRef card_ref_;
				state::Cards::Card & card_;
			};
		}
	}
}