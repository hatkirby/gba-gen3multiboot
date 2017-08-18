/*
 * Copyright (C) 2017 hatkirby
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include <gba.h>
#include "gamedata.h"
#include "link.h"
#include "serialize.h"

int main(void)
{
  // This possibly increases stability, I don't rightly know, this is all black
  // magic, will test more later.
  REG_IME = 0;

  initializeLink();

  // Identify the host game.
  if (GAME_RUBY)
  {
    sendS32(1);
  } else if (GAME_SAPP)
  {
    sendS32(2);
  } else if (GAME_FR)
  {
    sendS32(3);
  } else if (GAME_LG)
  {
    sendS32(4);
  } else if (GAME_EM)
  {
    sendS32(5);
  } else {
    sendS32(-1);
    waitForAck();

    return 0;
  }

  waitForAck();

  // Get access to save data.
  struct GameData gameData;

  if (!initSaveData(&gameData))
  {
    // Unsupported game version.
    sendS32(-1);
    waitForAck();

    return 0;
  }

  sendS32(1);
  waitForAck();

  // Send trainer name.
  u8* trainerName = 0;

  if (GAME_RS)
  {
    trainerName = gameData.SaveBlock2->rs.playerName;
  } else if (GAME_FRLG)
  {
    trainerName = gameData.SaveBlock2->frlg.playerName;
  } else if (GAME_EM)
  {
    trainerName = gameData.SaveBlock2->e.playerName;
  }

  u32 tn1 =
      (trainerName[0] << 24)
    | (trainerName[1] << 16)
    | (trainerName[2] << 8)
    | (trainerName[3]);

  u32 tn2 =
      (trainerName[4] << 24)
    | (trainerName[5] << 16)
    | (trainerName[6] << 8)
    | (trainerName[7]);

  sendU32(tn1);
  waitForAck();

  sendU32(tn2);
  waitForAck();

  // Send trainer ID.
  u8* trainerId = 0;
  if (GAME_RS)
  {
    trainerId = gameData.SaveBlock2->rs.playerTrainerId;
  } else if (GAME_FRLG)
  {
    trainerId = gameData.SaveBlock2->frlg.playerTrainerId;
  } else if (GAME_EM)
  {
    trainerId = gameData.SaveBlock2->e.playerTrainerId;
  }

  u16 trainerIdNum =
      (trainerId[1] << 8)
    | (trainerId[0]);

  u16 secretIdNum =
      (trainerId[3] << 8)
    | (trainerId[2]);

  sendU32(trainerIdNum);
  waitForAck();

  // Does the player want to import this game?
  if (waitForResponse() == 0)
  {
    return 0;
  }

  // Send Pokédex data
  u8* pokedexSeen = 0;
  if (GAME_RS)
  {
    pokedexSeen = gameData.SaveBlock2->rs.pokedex.seen;
  } else if (GAME_FRLG)
  {
    pokedexSeen = gameData.SaveBlock2->frlg.pokedex.seen;
  } else if (GAME_EM)
  {
    pokedexSeen = gameData.SaveBlock2->e.pokedex.seen;
  }

  for (int i=0; i<13; i++)
  {
    u32 psi =
        (pokedexSeen[i*4]   << 24)
      | (pokedexSeen[i*4+1] << 16)
      | (pokedexSeen[i*4+2] << 8)
      | (pokedexSeen[i*4+3]);

    directSendU32(psi);
  }

  u8* pokedexCaught = 0;
  if (GAME_RS)
  {
    pokedexCaught = gameData.SaveBlock2->rs.pokedex.owned;
  } else if (GAME_FRLG)
  {
    pokedexCaught = gameData.SaveBlock2->frlg.pokedex.owned;
  } else if (GAME_EM)
  {
    pokedexCaught = gameData.SaveBlock2->e.pokedex.owned;
  }

  for (int i=0; i<13; i++)
  {
    u32 psi =
        (pokedexCaught[i*4]   << 24)
      | (pokedexCaught[i*4+1] << 16)
      | (pokedexCaught[i*4+2] << 8)
      | (pokedexCaught[i*4+3]);

    directSendU32(psi);
  }

  // Start sending over party pokémon.
  struct Pokemon* playerParty = 0;
  if (GAME_RS)
  {
    playerParty = gameData.SaveBlock1->rs.playerParty;
  } else if (GAME_FRLG)
  {
    playerParty = gameData.SaveBlock1->frlg.playerParty;
  } else if (GAME_EM)
  {
    playerParty = gameData.SaveBlock1->e.playerParty;
  }

  waitForResponse();

  u32 partyCount = 1;

  sendU32(partyCount);
  waitForAck();

  for (int pki=0; pki<partyCount; pki++)
  {
    struct Pokemon* pkm = (playerParty + pki);
    struct BoxPokemon* bpkm = &(pkm->box);

    struct PokemonIntermediate pki;

    PokemonIntermediateInit(&pki, bpkm, trainerIdNum, secretIdNum, &gameData);
    PokemonIntermediateStream(&pki);
  }
}
