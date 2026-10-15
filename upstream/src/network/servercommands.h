// 19720b329745c7cc669e4d5792ea05ce
// This file has been automatically generated. Do not edit by hand.
#pragma once
#include "actor.h"
#include "zstring.h"
#include "network_enums.h"
#include "networkshared.h"
#include "sv_commands.h"
#include "joinqueue.h"
#include "network/netcommand.h"

bool CLIENT_ParseServerCommand( SVC header, BYTESTREAM_s *bytestream );
bool CLIENT_ParseExtendedServerCommand( SVC2 header, BYTESTREAM_s *bytestream );

namespace ServerCommands
{
	struct CVar
	{
		FName name;
		FString value;
	};

	struct JoinSlot
	{
		int player;
		int team;
	};

	struct MapRotationEntry
	{
		FString name;
		int isUsed;
		int minPlayers;
		int maxPlayers;
	};

	struct Medal
	{
		int index;
		unsigned int count;
	};

	class BaseServerCommand
	{
	public:
		virtual void PrintMissingParameters() const = 0;
		virtual bool AllParametersInitialized() const = 0;
		virtual NetCommand BuildNetCommand() const = 0;
		virtual void Execute() = 0;

		inline void sendCommandToClients( int playerExtra = MAXPLAYERS, ServerCommandFlags flags = 0 )
		{
			BuildNetCommand().sendCommandToClients( playerExtra, flags );
		}
	};

	class Header : public BaseServerCommand
	{
	public:
		Header() :
			_sequenceInitialized( false ) {}
		void SetSequence( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sequenceInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sequenceInitialized == false )
				Printf( "Missing: sequence\n" );
		}

	protected:
		int sequence;
		bool _sequenceInitialized;
	};

	class Ping : public BaseServerCommand
	{
	public:
		Ping() :
			_timeInitialized( false ) {}
		void SetTime( unsigned int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _timeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _timeInitialized == false )
				Printf( "Missing: time\n" );
		}

	protected:
		unsigned int time;
		bool _timeInitialized;
	};

	class BeginSnapshot : public BaseServerCommand
	{
	public:
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return true;
		}
		void PrintMissingParameters() const
		{
		}

	protected:
	};

	class EndSnapshot : public BaseServerCommand
	{
	public:
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return true;
		}
		void PrintMissingParameters() const
		{
		}

	protected:
	};

	class FullUpdateCompleted : public BaseServerCommand
	{
	public:
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return true;
		}
		void PrintMissingParameters() const
		{
		}

	protected:
	};

	class MapLoad : public BaseServerCommand
	{
	public:
		MapLoad() :
			_mapNameInitialized( false ),
			_currentPositionInitialized( false ) {}
		void SetMapName( const FString & value );
		void SetCurrentPosition( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _mapNameInitialized
				&& _currentPositionInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _mapNameInitialized == false )
				Printf( "Missing: mapName\n" );
			if ( _currentPositionInitialized == false )
				Printf( "Missing: currentPosition\n" );
		}

	protected:
		FString mapName;
		int currentPosition;
		bool _mapNameInitialized;
		bool _currentPositionInitialized;
	};

	class MapNew : public BaseServerCommand
	{
	public:
		MapNew() :
			_mapNameInitialized( false ) {}
		void SetMapName( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _mapNameInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _mapNameInitialized == false )
				Printf( "Missing: mapName\n" );
		}

	protected:
		FString mapName;
		bool _mapNameInitialized;
	};

	class MapExit : public BaseServerCommand
	{
	public:
		MapExit() :
			_positionInitialized( false ),
			_nextMapInitialized( false ),
			_changeFlagsInitialized( false ) {}
		void SetPosition( int value );
		void SetNextMap( const FString & value );
		void SetChangeFlags( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _positionInitialized
				&& _nextMapInitialized
				&& _changeFlagsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _positionInitialized == false )
				Printf( "Missing: position\n" );
			if ( _nextMapInitialized == false )
				Printf( "Missing: nextMap\n" );
			if ( _changeFlagsInitialized == false )
				Printf( "Missing: changeFlags\n" );
		}

	protected:
		int position;
		FString nextMap;
		int changeFlags;
		bool _positionInitialized;
		bool _nextMapInitialized;
		bool _changeFlagsInitialized;
	};

	class MapAuthenticate : public BaseServerCommand
	{
	public:
		MapAuthenticate() :
			_mapNameInitialized( false ) {}
		void SetMapName( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _mapNameInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _mapNameInitialized == false )
				Printf( "Missing: mapName\n" );
		}

	protected:
		FString mapName;
		bool _mapNameInitialized;
	};

	class SetMapTime : public BaseServerCommand
	{
	public:
		SetMapTime() :
			_timeInitialized( false ) {}
		void SetTime( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _timeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _timeInitialized == false )
				Printf( "Missing: time\n" );
		}

	protected:
		int time;
		bool _timeInitialized;
	};

	class SetMapNumKilledMonsters : public BaseServerCommand
	{
	public:
		SetMapNumKilledMonsters() :
			_killedMonstersInitialized( false ) {}
		void SetKilledMonsters( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _killedMonstersInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _killedMonstersInitialized == false )
				Printf( "Missing: killedMonsters\n" );
		}

	protected:
		int killedMonsters;
		bool _killedMonstersInitialized;
	};

	class SetMapNumFoundItems : public BaseServerCommand
	{
	public:
		SetMapNumFoundItems() :
			_foundItemsInitialized( false ) {}
		void SetFoundItems( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _foundItemsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _foundItemsInitialized == false )
				Printf( "Missing: foundItems\n" );
		}

	protected:
		int foundItems;
		bool _foundItemsInitialized;
	};

	class SetMapNumFoundSecrets : public BaseServerCommand
	{
	public:
		SetMapNumFoundSecrets() :
			_foundSecretsInitialized( false ) {}
		void SetFoundSecrets( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _foundSecretsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _foundSecretsInitialized == false )
				Printf( "Missing: foundSecrets\n" );
		}

	protected:
		int foundSecrets;
		bool _foundSecretsInitialized;
	};

	class SetMapNumTotalMonsters : public BaseServerCommand
	{
	public:
		SetMapNumTotalMonsters() :
			_totalMonstersInitialized( false ) {}
		void SetTotalMonsters( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _totalMonstersInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _totalMonstersInitialized == false )
				Printf( "Missing: totalMonsters\n" );
		}

	protected:
		int totalMonsters;
		bool _totalMonstersInitialized;
	};

	class SetMapNumTotalItems : public BaseServerCommand
	{
	public:
		SetMapNumTotalItems() :
			_totalItemsInitialized( false ) {}
		void SetTotalItems( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _totalItemsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _totalItemsInitialized == false )
				Printf( "Missing: totalItems\n" );
		}

	protected:
		int totalItems;
		bool _totalItemsInitialized;
	};

	class SetMapNumTotalSecrets : public BaseServerCommand
	{
	public:
		SetMapNumTotalSecrets() :
			_totalSecretsInitialized( false ) {}
		void SetTotalSecrets( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _totalSecretsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _totalSecretsInitialized == false )
				Printf( "Missing: totalSecrets\n" );
		}

	protected:
		int totalSecrets;
		bool _totalSecretsInitialized;
	};

	class SetMapMusic : public BaseServerCommand
	{
	public:
		SetMapMusic() :
			_musicInitialized( false ),
			_orderInitialized( false ) {}
		void SetMusic( const FString & value );
		void SetOrder( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _musicInitialized
				&& _orderInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _musicInitialized == false )
				Printf( "Missing: music\n" );
			if ( _orderInitialized == false )
				Printf( "Missing: order\n" );
		}

	protected:
		FString music;
		int order;
		bool _musicInitialized;
		bool _orderInitialized;
	};

	class SetMapSky : public BaseServerCommand
	{
	public:
		SetMapSky() :
			_sky1Initialized( false ),
			_sky2Initialized( false ) {}
		void SetSky1( const FString & value );
		void SetSky2( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sky1Initialized
				&& _sky2Initialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sky1Initialized == false )
				Printf( "Missing: sky1\n" );
			if ( _sky2Initialized == false )
				Printf( "Missing: sky2\n" );
		}

	protected:
		FString sky1;
		FString sky2;
		bool _sky1Initialized;
		bool _sky2Initialized;
	};

	class SetMapSkyScrollSpeed : public BaseServerCommand
	{
	public:
		SetMapSkyScrollSpeed() :
			_isSky1Initialized( false ),
			_valueInitialized( false ) {}
		void SetIsSky1( bool value );
		void SetValue( float value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _isSky1Initialized
				&& _valueInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _isSky1Initialized == false )
				Printf( "Missing: isSky1\n" );
			if ( _valueInitialized == false )
				Printf( "Missing: value\n" );
		}

	protected:
		bool isSky1;
		float value;
		bool _isSky1Initialized;
		bool _valueInitialized;
	};

	class SecretFound : public BaseServerCommand
	{
	public:
		SecretFound() :
			_actorInitialized( false ),
			_secretFlagsInitialized( false ) {}
		void SetActor( AActor * value );
		void SetSecretFlags( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _secretFlagsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _secretFlagsInitialized == false )
				Printf( "Missing: secretFlags\n" );
		}

	protected:
		AActor *actor;
		int secretFlags;
		bool _actorInitialized;
		bool _secretFlagsInitialized;
	};

	class SecretMarkSectorFound : public BaseServerCommand
	{
	public:
		SecretMarkSectorFound() :
			_sectorInitialized( false ) {}
		void SetSector( sector_t * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
		}

	protected:
		sector_t *sector;
		bool _sectorInitialized;
	};

	class DoScroller : public BaseServerCommand
	{
	public:
		DoScroller() :
			_typeInitialized( false ),
			_xInitialized( false ),
			_yInitialized( false ),
			_affecteeInitialized( false ),
			_accelInitialized( false ),
			_hasControlInitialized( false ),
			_hasPosInitialized( false ),
			_sectorInitialized( false ),
			_posInitialized( false ) {}
		void SetType( int value );
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetAffectee( int value );
		void SetAccel( bool value );
		void SetHasControl( bool value );
		void SetHasPos( bool value );
		void SetSector( sector_t * value );
		void SetPos( int value );
		bool ContainsPos() const;
		bool ContainsSector() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _typeInitialized
				&& _xInitialized
				&& _yInitialized
				&& _affecteeInitialized
				&& _accelInitialized
				&& _hasControlInitialized
				&& _hasPosInitialized
				&& _sectorInitialized
				&& _posInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _typeInitialized == false )
				Printf( "Missing: type\n" );
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _affecteeInitialized == false )
				Printf( "Missing: affectee\n" );
			if ( _accelInitialized == false )
				Printf( "Missing: accel\n" );
			if ( _hasControlInitialized == false )
				Printf( "Missing: hasControl\n" );
			if ( _hasPosInitialized == false )
				Printf( "Missing: hasPos\n" );
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _posInitialized == false )
				Printf( "Missing: pos\n" );
		}

	protected:
		int type;
		fixed_t x;
		fixed_t y;
		int affectee;
		bool accel;
		bool hasControl;
		bool hasPos;
		sector_t *sector;
		int pos;
		bool _typeInitialized;
		bool _xInitialized;
		bool _yInitialized;
		bool _affecteeInitialized;
		bool _accelInitialized;
		bool _hasControlInitialized;
		bool _hasPosInitialized;
		bool _sectorInitialized;
		bool _posInitialized;
	};

	class SpawnPlayer : public BaseServerCommand
	{
	public:
		SpawnPlayer() :
			_playerInitialized( false ),
			_priorStateInitialized( false ),
			_isBotInitialized( false ),
			_isSpectatingInitialized( false ),
			_isDeadSpectatorInitialized( false ),
			_isMorphedInitialized( false ),
			_netidInitialized( false ),
			_angleInitialized( false ),
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_playerClassInitialized( false ),
			_morphStyleInitialized( false ),
			_morphedClassInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetPriorState( int value );
		void SetIsBot( bool value );
		void SetIsSpectating( bool value );
		void SetIsDeadSpectator( bool value );
		void SetIsMorphed( bool value );
		void SetNetid( unsigned short value );
		void SetAngle( angle_t value );
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetPlayerClass( int value );
		void SetMorphStyle( int value );
		void SetMorphedClass( const PClass * value );
		bool CheckIsMorphed() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _priorStateInitialized
				&& _isBotInitialized
				&& _isSpectatingInitialized
				&& _isDeadSpectatorInitialized
				&& _isMorphedInitialized
				&& _netidInitialized
				&& _angleInitialized
				&& _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _playerClassInitialized
				&& _morphStyleInitialized
				&& _morphedClassInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _priorStateInitialized == false )
				Printf( "Missing: priorState\n" );
			if ( _isBotInitialized == false )
				Printf( "Missing: isBot\n" );
			if ( _isSpectatingInitialized == false )
				Printf( "Missing: isSpectating\n" );
			if ( _isDeadSpectatorInitialized == false )
				Printf( "Missing: isDeadSpectator\n" );
			if ( _isMorphedInitialized == false )
				Printf( "Missing: isMorphed\n" );
			if ( _netidInitialized == false )
				Printf( "Missing: netid\n" );
			if ( _angleInitialized == false )
				Printf( "Missing: angle\n" );
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _playerClassInitialized == false )
				Printf( "Missing: playerClass\n" );
			if ( _morphStyleInitialized == false )
				Printf( "Missing: morphStyle\n" );
			if ( _morphedClassInitialized == false )
				Printf( "Missing: morphedClass\n" );
		}

	protected:
		player_t *player;
		int priorState;
		bool isBot;
		bool isSpectating;
		bool isDeadSpectator;
		bool isMorphed;
		unsigned short netid;
		angle_t angle;
		fixed_t x;
		fixed_t y;
		fixed_t z;
		int playerClass;
		int morphStyle;
		const PClass *morphedClass;
		bool _playerInitialized;
		bool _priorStateInitialized;
		bool _isBotInitialized;
		bool _isSpectatingInitialized;
		bool _isDeadSpectatorInitialized;
		bool _isMorphedInitialized;
		bool _netidInitialized;
		bool _angleInitialized;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _playerClassInitialized;
		bool _morphStyleInitialized;
		bool _morphedClassInitialized;
	};

	class MovePlayer : public BaseServerCommand
	{
	public:
		MovePlayer() :
			_playerInitialized( false ),
			_flagsInitialized( false ),
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_angleInitialized( false ),
			_velxInitialized( false ),
			_velyInitialized( false ),
			_velzInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetFlags( int value );
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetAngle( angle_t value );
		void SetVelx( fixed_t value );
		void SetVely( fixed_t value );
		void SetVelz( fixed_t value );
		bool IsMovingX() const;
		bool IsMovingY() const;
		bool IsMovingZ() const;
		bool IsVisible() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _flagsInitialized
				&& _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _angleInitialized
				&& _velxInitialized
				&& _velyInitialized
				&& _velzInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _flagsInitialized == false )
				Printf( "Missing: flags\n" );
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _angleInitialized == false )
				Printf( "Missing: angle\n" );
			if ( _velxInitialized == false )
				Printf( "Missing: velx\n" );
			if ( _velyInitialized == false )
				Printf( "Missing: vely\n" );
			if ( _velzInitialized == false )
				Printf( "Missing: velz\n" );
		}

	protected:
		player_t *player;
		int flags;
		fixed_t x;
		fixed_t y;
		fixed_t z;
		angle_t angle;
		fixed_t velx;
		fixed_t vely;
		fixed_t velz;
		bool _playerInitialized;
		bool _flagsInitialized;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _angleInitialized;
		bool _velxInitialized;
		bool _velyInitialized;
		bool _velzInitialized;
	};

	class DamagePlayer : public BaseServerCommand
	{
	public:
		DamagePlayer() :
			_playerInitialized( false ),
			_healthInitialized( false ),
			_armorInitialized( false ),
			_attackerInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetHealth( int value );
		void SetArmor( int value );
		void SetAttacker( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _healthInitialized
				&& _armorInitialized
				&& _attackerInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _healthInitialized == false )
				Printf( "Missing: health\n" );
			if ( _armorInitialized == false )
				Printf( "Missing: armor\n" );
			if ( _attackerInitialized == false )
				Printf( "Missing: attacker\n" );
		}

	protected:
		player_t *player;
		int health;
		int armor;
		AActor *attacker;
		bool _playerInitialized;
		bool _healthInitialized;
		bool _armorInitialized;
		bool _attackerInitialized;
	};

	class DamagePlayerWithType : public BaseServerCommand
	{
	public:
		DamagePlayerWithType() :
			_playerInitialized( false ),
			_healthInitialized( false ),
			_armorInitialized( false ),
			_damageTypeInitialized( false ),
			_attackerInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetHealth( int value );
		void SetArmor( int value );
		void SetDamageType( const FString & value );
		void SetAttacker( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _healthInitialized
				&& _armorInitialized
				&& _damageTypeInitialized
				&& _attackerInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _healthInitialized == false )
				Printf( "Missing: health\n" );
			if ( _armorInitialized == false )
				Printf( "Missing: armor\n" );
			if ( _damageTypeInitialized == false )
				Printf( "Missing: damageType\n" );
			if ( _attackerInitialized == false )
				Printf( "Missing: attacker\n" );
		}

	protected:
		player_t *player;
		int health;
		int armor;
		FString damageType;
		AActor *attacker;
		bool _playerInitialized;
		bool _healthInitialized;
		bool _armorInitialized;
		bool _damageTypeInitialized;
		bool _attackerInitialized;
	};

	class KillPlayer : public BaseServerCommand
	{
	public:
		KillPlayer() :
			_playerInitialized( false ),
			_sourceInitialized( false ),
			_inflictorInitialized( false ),
			_healthInitialized( false ),
			_MODInitialized( false ),
			_damageTypeInitialized( false ),
			_weaponTypeInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetSource( AActor * value );
		void SetInflictor( AActor * value );
		void SetHealth( int value );
		void SetMOD( const FString & value );
		void SetDamageType( const FString & value );
		void SetWeaponType( const PClass * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _sourceInitialized
				&& _inflictorInitialized
				&& _healthInitialized
				&& _MODInitialized
				&& _damageTypeInitialized
				&& _weaponTypeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _sourceInitialized == false )
				Printf( "Missing: source\n" );
			if ( _inflictorInitialized == false )
				Printf( "Missing: inflictor\n" );
			if ( _healthInitialized == false )
				Printf( "Missing: health\n" );
			if ( _MODInitialized == false )
				Printf( "Missing: MOD\n" );
			if ( _damageTypeInitialized == false )
				Printf( "Missing: damageType\n" );
			if ( _weaponTypeInitialized == false )
				Printf( "Missing: weaponType\n" );
		}

	protected:
		player_t *player;
		AActor *source;
		AActor *inflictor;
		int health;
		FName MOD;
		FString damageType;
		const PClass *weaponType;
		bool _playerInitialized;
		bool _sourceInitialized;
		bool _inflictorInitialized;
		bool _healthInitialized;
		bool _MODInitialized;
		bool _damageTypeInitialized;
		bool _weaponTypeInitialized;
	};

	class SetPlayerHealth : public BaseServerCommand
	{
	public:
		SetPlayerHealth() :
			_playerInitialized( false ),
			_healthInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetHealth( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _healthInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _healthInitialized == false )
				Printf( "Missing: health\n" );
		}

	protected:
		player_t *player;
		int health;
		bool _playerInitialized;
		bool _healthInitialized;
	};

	class SetPlayerArmor : public BaseServerCommand
	{
	public:
		SetPlayerArmor() :
			_playerInitialized( false ),
			_armorAmountInitialized( false ),
			_armorIconInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetArmorAmount( int value );
		void SetArmorIcon( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _armorAmountInitialized
				&& _armorIconInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _armorAmountInitialized == false )
				Printf( "Missing: armorAmount\n" );
			if ( _armorIconInitialized == false )
				Printf( "Missing: armorIcon\n" );
		}

	protected:
		player_t *player;
		int armorAmount;
		FString armorIcon;
		bool _playerInitialized;
		bool _armorAmountInitialized;
		bool _armorIconInitialized;
	};

	class SetPlayerState : public BaseServerCommand
	{
	public:
		SetPlayerState() :
			_playerInitialized( false ),
			_stateInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetState( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _stateInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _stateInitialized == false )
				Printf( "Missing: state\n" );
		}

	protected:
		player_t *player;
		int state;
		bool _playerInitialized;
		bool _stateInitialized;
	};

	class SetPlayerUserInfo : public BaseServerCommand
	{
	public:
		SetPlayerUserInfo() :
			_playerInitialized( false ),
			_cvarsInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetCvars( const TArray<struct CVar> & value );
		void PushToCvars(const struct CVar & value)
		{
			cvars.Push(value);
			_cvarsInitialized = true;
		}
		bool PopFromCvars(struct CVar& value)
		{
			return cvars.Pop(value);
		}
		void ClearCvars()
		{
			cvars.Clear();
		}
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _cvarsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _cvarsInitialized == false )
				Printf( "Missing: cvars\n" );
		}

	protected:
		player_t *player;
		TArray<struct CVar> cvars;
		bool _playerInitialized;
		bool _cvarsInitialized;
	};

	class SetPlayerCountry : public BaseServerCommand
	{
	public:
		SetPlayerCountry() :
			_playerInitialized( false ),
			_countryInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetCountry( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _countryInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _countryInitialized == false )
				Printf( "Missing: country\n" );
		}

	protected:
		player_t *player;
		int country;
		bool _playerInitialized;
		bool _countryInitialized;
	};

	class SetPlayerAccountName : public BaseServerCommand
	{
	public:
		SetPlayerAccountName() :
			_playerInitialized( false ),
			_accountNameInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetAccountName( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _accountNameInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _accountNameInitialized == false )
				Printf( "Missing: accountName\n" );
		}

	protected:
		player_t *player;
		FString accountName;
		bool _playerInitialized;
		bool _accountNameInitialized;
	};

	class SetPlayerFrags : public BaseServerCommand
	{
	public:
		SetPlayerFrags() :
			_playerInitialized( false ),
			_fragCountInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetFragCount( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _fragCountInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _fragCountInitialized == false )
				Printf( "Missing: fragCount\n" );
		}

	protected:
		player_t *player;
		int fragCount;
		bool _playerInitialized;
		bool _fragCountInitialized;
	};

	class SetPlayerPoints : public BaseServerCommand
	{
	public:
		SetPlayerPoints() :
			_playerInitialized( false ),
			_pointCountInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetPointCount( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _pointCountInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _pointCountInitialized == false )
				Printf( "Missing: pointCount\n" );
		}

	protected:
		player_t *player;
		int pointCount;
		bool _playerInitialized;
		bool _pointCountInitialized;
	};

	class SetPlayerWins : public BaseServerCommand
	{
	public:
		SetPlayerWins() :
			_playerInitialized( false ),
			_winsInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetWins( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _winsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _winsInitialized == false )
				Printf( "Missing: wins\n" );
		}

	protected:
		player_t *player;
		int wins;
		bool _playerInitialized;
		bool _winsInitialized;
	};

	class SetPlayerDeaths : public BaseServerCommand
	{
	public:
		SetPlayerDeaths() :
			_playerInitialized( false ),
			_deathsInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetDeaths( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _deathsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _deathsInitialized == false )
				Printf( "Missing: deaths\n" );
		}

	protected:
		player_t *player;
		int deaths;
		bool _playerInitialized;
		bool _deathsInitialized;
	};

	class SetPlayerKillCount : public BaseServerCommand
	{
	public:
		SetPlayerKillCount() :
			_playerInitialized( false ),
			_killCountInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetKillCount( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _killCountInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _killCountInitialized == false )
				Printf( "Missing: killCount\n" );
		}

	protected:
		player_t *player;
		int killCount;
		bool _playerInitialized;
		bool _killCountInitialized;
	};

	class SetPlayerStatus : public BaseServerCommand
	{
	public:
		SetPlayerStatus() :
			_playerInitialized( false ),
			_statusesInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetStatuses( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _statusesInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _statusesInitialized == false )
				Printf( "Missing: statuses\n" );
		}

	protected:
		player_t *player;
		int statuses;
		bool _playerInitialized;
		bool _statusesInitialized;
	};

	class SetPlayerTeam : public BaseServerCommand
	{
	public:
		SetPlayerTeam() :
			_playerInitialized( false ),
			_teamInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetTeam( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _teamInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _teamInitialized == false )
				Printf( "Missing: team\n" );
		}

	protected:
		player_t *player;
		int team;
		bool _playerInitialized;
		bool _teamInitialized;
	};

	class SetPlayerCamera : public BaseServerCommand
	{
	public:
		SetPlayerCamera() :
			_cameraInitialized( false ),
			_revertPleaseInitialized( false ) {}
		void SetCamera( AActor * value );
		void SetRevertPlease( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _cameraInitialized
				&& _revertPleaseInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _cameraInitialized == false )
				Printf( "Missing: camera\n" );
			if ( _revertPleaseInitialized == false )
				Printf( "Missing: revertPlease\n" );
		}

	protected:
		AActor *camera;
		bool revertPlease;
		bool _cameraInitialized;
		bool _revertPleaseInitialized;
	};

	class SetPlayerPoisonCount : public BaseServerCommand
	{
	public:
		SetPlayerPoisonCount() :
			_playerInitialized( false ),
			_poisonCountInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetPoisonCount( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _poisonCountInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _poisonCountInitialized == false )
				Printf( "Missing: poisonCount\n" );
		}

	protected:
		player_t *player;
		int poisonCount;
		bool _playerInitialized;
		bool _poisonCountInitialized;
	};

	class SetPlayerAmmoCapacity : public BaseServerCommand
	{
	public:
		SetPlayerAmmoCapacity() :
			_playerInitialized( false ),
			_ammoTypeInitialized( false ),
			_maxAmountInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetAmmoType( const PClass * value );
		void SetMaxAmount( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _ammoTypeInitialized
				&& _maxAmountInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _ammoTypeInitialized == false )
				Printf( "Missing: ammoType\n" );
			if ( _maxAmountInitialized == false )
				Printf( "Missing: maxAmount\n" );
		}

	protected:
		player_t *player;
		const PClass *ammoType;
		int maxAmount;
		bool _playerInitialized;
		bool _ammoTypeInitialized;
		bool _maxAmountInitialized;
	};

	class SetPlayerCheats : public BaseServerCommand
	{
	public:
		SetPlayerCheats() :
			_playerInitialized( false ),
			_cheatsInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetCheats( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _cheatsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _cheatsInitialized == false )
				Printf( "Missing: cheats\n" );
		}

	protected:
		player_t *player;
		int cheats;
		bool _playerInitialized;
		bool _cheatsInitialized;
	};

	class SetPlayerPendingWeapon : public BaseServerCommand
	{
	public:
		SetPlayerPendingWeapon() :
			_playerInitialized( false ),
			_weaponTypeInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetWeaponType( const PClass * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _weaponTypeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _weaponTypeInitialized == false )
				Printf( "Missing: weaponType\n" );
		}

	protected:
		player_t *player;
		const PClass *weaponType;
		bool _playerInitialized;
		bool _weaponTypeInitialized;
	};

	class SetPlayerPSprite : public BaseServerCommand
	{
	public:
		SetPlayerPSprite() :
			_playerInitialized( false ),
			_stateOwnerInitialized( false ),
			_offsetInitialized( false ),
			_positionInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetStateOwner( const PClass * value );
		void SetOffset( int value );
		void SetPosition( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _stateOwnerInitialized
				&& _offsetInitialized
				&& _positionInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _stateOwnerInitialized == false )
				Printf( "Missing: stateOwner\n" );
			if ( _offsetInitialized == false )
				Printf( "Missing: offset\n" );
			if ( _positionInitialized == false )
				Printf( "Missing: position\n" );
		}

	protected:
		player_t *player;
		const PClass *stateOwner;
		int offset;
		int position;
		bool _playerInitialized;
		bool _stateOwnerInitialized;
		bool _offsetInitialized;
		bool _positionInitialized;
	};

	class SetPlayerBlend : public BaseServerCommand
	{
	public:
		SetPlayerBlend() :
			_playerInitialized( false ),
			_blendRInitialized( false ),
			_blendGInitialized( false ),
			_blendBInitialized( false ),
			_blendAInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetBlendR( float value );
		void SetBlendG( float value );
		void SetBlendB( float value );
		void SetBlendA( float value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _blendRInitialized
				&& _blendGInitialized
				&& _blendBInitialized
				&& _blendAInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _blendRInitialized == false )
				Printf( "Missing: blendR\n" );
			if ( _blendGInitialized == false )
				Printf( "Missing: blendG\n" );
			if ( _blendBInitialized == false )
				Printf( "Missing: blendB\n" );
			if ( _blendAInitialized == false )
				Printf( "Missing: blendA\n" );
		}

	protected:
		player_t *player;
		float blendR;
		float blendG;
		float blendB;
		float blendA;
		bool _playerInitialized;
		bool _blendRInitialized;
		bool _blendGInitialized;
		bool _blendBInitialized;
		bool _blendAInitialized;
	};

	class SetPlayerMaxHealth : public BaseServerCommand
	{
	public:
		SetPlayerMaxHealth() :
			_playerInitialized( false ),
			_maxHealthInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetMaxHealth( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _maxHealthInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _maxHealthInitialized == false )
				Printf( "Missing: maxHealth\n" );
		}

	protected:
		player_t *player;
		int maxHealth;
		bool _playerInitialized;
		bool _maxHealthInitialized;
	};

	class SetPlayerLivesLeft : public BaseServerCommand
	{
	public:
		SetPlayerLivesLeft() :
			_playerInitialized( false ),
			_livesLeftInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetLivesLeft( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _livesLeftInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _livesLeftInitialized == false )
				Printf( "Missing: livesLeft\n" );
		}

	protected:
		player_t *player;
		int livesLeft;
		bool _playerInitialized;
		bool _livesLeftInitialized;
	};

	class SetPlayerACSSkin : public BaseServerCommand
	{
	public:
		SetPlayerACSSkin() :
			_playerInitialized( false ),
			_skinNameInitialized( false ),
			_overrideWeaponSkinInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetSkinName( FName value );
		void SetOverrideWeaponSkin( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _skinNameInitialized
				&& _overrideWeaponSkinInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _skinNameInitialized == false )
				Printf( "Missing: skinName\n" );
			if ( _overrideWeaponSkinInitialized == false )
				Printf( "Missing: overrideWeaponSkin\n" );
		}

	protected:
		player_t *player;
		FName skinName;
		bool overrideWeaponSkin;
		bool _playerInitialized;
		bool _skinNameInitialized;
		bool _overrideWeaponSkinInitialized;
	};

	class UpdatePlayerPing : public BaseServerCommand
	{
	public:
		UpdatePlayerPing() :
			_playerInitialized( false ),
			_pingInitialized( false ),
			_connectionStrengthInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetPing( unsigned int value );
		void SetConnectionStrength( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _pingInitialized
				&& _connectionStrengthInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _pingInitialized == false )
				Printf( "Missing: ping\n" );
			if ( _connectionStrengthInitialized == false )
				Printf( "Missing: connectionStrength\n" );
		}

	protected:
		player_t *player;
		unsigned int ping;
		int connectionStrength;
		bool _playerInitialized;
		bool _pingInitialized;
		bool _connectionStrengthInitialized;
	};

	class UpdatePlayerExtraData : public BaseServerCommand
	{
	public:
		UpdatePlayerExtraData() :
			_playerInitialized( false ),
			_pitchInitialized( false ),
			_waterLevelInitialized( false ),
			_buttonsInitialized( false ),
			_viewZInitialized( false ),
			_bobInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetPitch( int value );
		void SetWaterLevel( int value );
		void SetButtons( int value );
		void SetViewZ( int value );
		void SetBob( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _pitchInitialized
				&& _waterLevelInitialized
				&& _buttonsInitialized
				&& _viewZInitialized
				&& _bobInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _pitchInitialized == false )
				Printf( "Missing: pitch\n" );
			if ( _waterLevelInitialized == false )
				Printf( "Missing: waterLevel\n" );
			if ( _buttonsInitialized == false )
				Printf( "Missing: buttons\n" );
			if ( _viewZInitialized == false )
				Printf( "Missing: viewZ\n" );
			if ( _bobInitialized == false )
				Printf( "Missing: bob\n" );
		}

	protected:
		player_t *player;
		int pitch;
		int waterLevel;
		int buttons;
		int viewZ;
		int bob;
		bool _playerInitialized;
		bool _pitchInitialized;
		bool _waterLevelInitialized;
		bool _buttonsInitialized;
		bool _viewZInitialized;
		bool _bobInitialized;
	};

	class UpdatePlayerTime : public BaseServerCommand
	{
	public:
		UpdatePlayerTime() :
			_playerInitialized( false ),
			_timeInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetTime( unsigned int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _timeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _timeInitialized == false )
				Printf( "Missing: time\n" );
		}

	protected:
		player_t *player;
		unsigned int time;
		bool _playerInitialized;
		bool _timeInitialized;
	};

	class MoveLocalPlayer : public BaseServerCommand
	{
	public:
		MoveLocalPlayer() :
			_clientTicOnServerEndInitialized( false ),
			_latestServerGameticInitialized( false ),
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_velxInitialized( false ),
			_velyInitialized( false ),
			_velzInitialized( false ) {}
		void SetClientTicOnServerEnd( unsigned int value );
		void SetLatestServerGametic( int value );
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetVelx( fixed_t value );
		void SetVely( fixed_t value );
		void SetVelz( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _clientTicOnServerEndInitialized
				&& _latestServerGameticInitialized
				&& _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _velxInitialized
				&& _velyInitialized
				&& _velzInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _clientTicOnServerEndInitialized == false )
				Printf( "Missing: clientTicOnServerEnd\n" );
			if ( _latestServerGameticInitialized == false )
				Printf( "Missing: latestServerGametic\n" );
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _velxInitialized == false )
				Printf( "Missing: velx\n" );
			if ( _velyInitialized == false )
				Printf( "Missing: vely\n" );
			if ( _velzInitialized == false )
				Printf( "Missing: velz\n" );
		}

	protected:
		unsigned int clientTicOnServerEnd;
		int latestServerGametic;
		fixed_t x;
		fixed_t y;
		fixed_t z;
		fixed_t velx;
		fixed_t vely;
		fixed_t velz;
		bool _clientTicOnServerEndInitialized;
		bool _latestServerGameticInitialized;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _velxInitialized;
		bool _velyInitialized;
		bool _velzInitialized;
	};

	class SetLocalPlayerJumpTics : public BaseServerCommand
	{
	public:
		SetLocalPlayerJumpTics() :
			_clientTicOnServerEndInitialized( false ),
			_jumpTicsInitialized( false ) {}
		void SetClientTicOnServerEnd( unsigned int value );
		void SetJumpTics( SBYTE value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _clientTicOnServerEndInitialized
				&& _jumpTicsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _clientTicOnServerEndInitialized == false )
				Printf( "Missing: clientTicOnServerEnd\n" );
			if ( _jumpTicsInitialized == false )
				Printf( "Missing: jumpTics\n" );
		}

	protected:
		unsigned int clientTicOnServerEnd;
		SBYTE jumpTics;
		bool _clientTicOnServerEndInitialized;
		bool _jumpTicsInitialized;
	};

	class SetLocalPlayerRespawnDelayTime : public BaseServerCommand
	{
	public:
		SetLocalPlayerRespawnDelayTime() :
			_respawnDelayTicsInitialized( false ),
			_spawnTelefraggedInitialized( false ) {}
		void SetRespawnDelayTics( unsigned int value );
		void SetSpawnTelefragged( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _respawnDelayTicsInitialized
				&& _spawnTelefraggedInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _respawnDelayTicsInitialized == false )
				Printf( "Missing: respawnDelayTics\n" );
			if ( _spawnTelefraggedInitialized == false )
				Printf( "Missing: spawnTelefragged\n" );
		}

	protected:
		unsigned int respawnDelayTics;
		bool spawnTelefragged;
		bool _respawnDelayTicsInitialized;
		bool _spawnTelefraggedInitialized;
	};

	class DisconnectPlayer : public BaseServerCommand
	{
	public:
		DisconnectPlayer() :
			_playerInitialized( false ) {}
		void SetPlayer( player_t * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
		}

	protected:
		player_t *player;
		bool _playerInitialized;
	};

	class SetConsolePlayer : public BaseServerCommand
	{
	public:
		SetConsolePlayer() :
			_playerNumberInitialized( false ) {}
		void SetPlayerNumber( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerNumberInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerNumberInitialized == false )
				Printf( "Missing: playerNumber\n" );
		}

	protected:
		int playerNumber;
		bool _playerNumberInitialized;
	};

	class ConsolePlayerKicked : public BaseServerCommand
	{
	public:
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return true;
		}
		void PrintMissingParameters() const
		{
		}

	protected:
	};

	class GivePlayerMedal : public BaseServerCommand
	{
	public:
		GivePlayerMedal() :
			_playerInitialized( false ),
			_medalInitialized( false ),
			_silentInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetMedal( int value );
		void SetSilent( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _medalInitialized
				&& _silentInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _medalInitialized == false )
				Printf( "Missing: medal\n" );
			if ( _silentInitialized == false )
				Printf( "Missing: silent\n" );
		}

	protected:
		player_t *player;
		int medal;
		bool silent;
		bool _playerInitialized;
		bool _medalInitialized;
		bool _silentInitialized;
	};

	class ResetAllPlayersFragcount : public BaseServerCommand
	{
	public:
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return true;
		}
		void PrintMissingParameters() const
		{
		}

	protected:
	};

	class PlayerIsSpectator : public BaseServerCommand
	{
	public:
		PlayerIsSpectator() :
			_playerInitialized( false ),
			_deadSpectatorInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetDeadSpectator( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _deadSpectatorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _deadSpectatorInitialized == false )
				Printf( "Missing: deadSpectator\n" );
		}

	protected:
		player_t *player;
		bool deadSpectator;
		bool _playerInitialized;
		bool _deadSpectatorInitialized;
	};

	class PlayerSay : public BaseServerCommand
	{
	public:
		PlayerSay() :
			_playerNumberInitialized( false ),
			_modeInitialized( false ),
			_messageInitialized( false ) {}
		void SetPlayerNumber( int value );
		void SetMode( int value );
		void SetMessage( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerNumberInitialized
				&& _modeInitialized
				&& _messageInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerNumberInitialized == false )
				Printf( "Missing: playerNumber\n" );
			if ( _modeInitialized == false )
				Printf( "Missing: mode\n" );
			if ( _messageInitialized == false )
				Printf( "Missing: message\n" );
		}

	protected:
		int playerNumber;
		int mode;
		FString message;
		bool _playerNumberInitialized;
		bool _modeInitialized;
		bool _messageInitialized;
	};

	class PlayerVoIPAudioPacket : public BaseServerCommand
	{
	public:
		PlayerVoIPAudioPacket() :
			_playerNumberInitialized( false ),
			_frameInitialized( false ),
			_audioInitialized( false ) {}
		void SetPlayerNumber( int value );
		void SetFrame( int value );
		void SetAudio( const BufferParameter & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerNumberInitialized
				&& _frameInitialized
				&& _audioInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerNumberInitialized == false )
				Printf( "Missing: playerNumber\n" );
			if ( _frameInitialized == false )
				Printf( "Missing: frame\n" );
			if ( _audioInitialized == false )
				Printf( "Missing: audio\n" );
		}

	protected:
		int playerNumber;
		int frame;
		BufferParameter audio;
		bool _playerNumberInitialized;
		bool _frameInitialized;
		bool _audioInitialized;
	};

	class PlayerTaunt : public BaseServerCommand
	{
	public:
		PlayerTaunt() :
			_playerInitialized( false ) {}
		void SetPlayer( player_t * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
		}

	protected:
		player_t *player;
		bool _playerInitialized;
	};

	class PlayerUseInventory : public BaseServerCommand
	{
	public:
		PlayerUseInventory() :
			_playerInitialized( false ),
			_itemTypeInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetItemType( const PClass * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _itemTypeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _itemTypeInitialized == false )
				Printf( "Missing: itemType\n" );
		}

	protected:
		player_t *player;
		const PClass *itemType;
		bool _playerInitialized;
		bool _itemTypeInitialized;
	};

	class PlayerDropInventory : public BaseServerCommand
	{
	public:
		PlayerDropInventory() :
			_playerInitialized( false ),
			_itemTypeInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetItemType( const PClass * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _itemTypeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _itemTypeInitialized == false )
				Printf( "Missing: itemType\n" );
		}

	protected:
		player_t *player;
		const PClass *itemType;
		bool _playerInitialized;
		bool _itemTypeInitialized;
	};

	class GiveWeaponHolder : public BaseServerCommand
	{
	public:
		GiveWeaponHolder() :
			_playerInitialized( false ),
			_pieceMaskInitialized( false ),
			_pieceWeaponInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetPieceMask( int value );
		void SetPieceWeapon( const PClass * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _pieceMaskInitialized
				&& _pieceWeaponInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _pieceMaskInitialized == false )
				Printf( "Missing: pieceMask\n" );
			if ( _pieceWeaponInitialized == false )
				Printf( "Missing: pieceWeapon\n" );
		}

	protected:
		player_t *player;
		int pieceMask;
		const PClass *pieceWeapon;
		bool _playerInitialized;
		bool _pieceMaskInitialized;
		bool _pieceWeaponInitialized;
	};

	class SetHexenArmorSlots : public BaseServerCommand
	{
	public:
		SetHexenArmorSlots() :
			_playerInitialized( false ),
			_slot0Initialized( false ),
			_slot1Initialized( false ),
			_slot2Initialized( false ),
			_slot3Initialized( false ),
			_slot4Initialized( false ) {}
		void SetPlayer( player_t * value );
		void SetSlot0( int value );
		void SetSlot1( int value );
		void SetSlot2( int value );
		void SetSlot3( int value );
		void SetSlot4( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _slot0Initialized
				&& _slot1Initialized
				&& _slot2Initialized
				&& _slot3Initialized
				&& _slot4Initialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _slot0Initialized == false )
				Printf( "Missing: slot0\n" );
			if ( _slot1Initialized == false )
				Printf( "Missing: slot1\n" );
			if ( _slot2Initialized == false )
				Printf( "Missing: slot2\n" );
			if ( _slot3Initialized == false )
				Printf( "Missing: slot3\n" );
			if ( _slot4Initialized == false )
				Printf( "Missing: slot4\n" );
		}

	protected:
		player_t *player;
		int slot0;
		int slot1;
		int slot2;
		int slot3;
		int slot4;
		bool _playerInitialized;
		bool _slot0Initialized;
		bool _slot1Initialized;
		bool _slot2Initialized;
		bool _slot3Initialized;
		bool _slot4Initialized;
	};

	class SendPlayerCommRule : public BaseServerCommand
	{
	public:
		SendPlayerCommRule() :
			_playerInitialized( false ),
			_ignoreChatInitialized( false ),
			_ignoreVoiceInitialized( false ),
			_sendVoIPChannelVolumeInitialized( false ),
			_ignoreChatTicksInitialized( false ),
			_ignoreVoiceTicksInitialized( false ),
			_VoIPChannelVolumeInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetIgnoreChat( bool value );
		void SetIgnoreVoice( bool value );
		void SetSendVoIPChannelVolume( bool value );
		void SetIgnoreChatTicks( int value );
		void SetIgnoreVoiceTicks( int value );
		void SetVoIPChannelVolume( float value );
		bool ContainsIgnoreChatTicks() const;
		bool ContainsIgnoreVoiceTicks() const;
		bool ContainsVoIPChannelVolume() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _ignoreChatInitialized
				&& _ignoreVoiceInitialized
				&& _sendVoIPChannelVolumeInitialized
				&& _ignoreChatTicksInitialized
				&& _ignoreVoiceTicksInitialized
				&& _VoIPChannelVolumeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _ignoreChatInitialized == false )
				Printf( "Missing: ignoreChat\n" );
			if ( _ignoreVoiceInitialized == false )
				Printf( "Missing: ignoreVoice\n" );
			if ( _sendVoIPChannelVolumeInitialized == false )
				Printf( "Missing: sendVoIPChannelVolume\n" );
			if ( _ignoreChatTicksInitialized == false )
				Printf( "Missing: ignoreChatTicks\n" );
			if ( _ignoreVoiceTicksInitialized == false )
				Printf( "Missing: ignoreVoiceTicks\n" );
			if ( _VoIPChannelVolumeInitialized == false )
				Printf( "Missing: VoIPChannelVolume\n" );
		}

	protected:
		player_t *player;
		bool ignoreChat;
		bool ignoreVoice;
		bool sendVoIPChannelVolume;
		int ignoreChatTicks;
		int ignoreVoiceTicks;
		float VoIPChannelVolume;
		bool _playerInitialized;
		bool _ignoreChatInitialized;
		bool _ignoreVoiceInitialized;
		bool _sendVoIPChannelVolumeInitialized;
		bool _ignoreChatTicksInitialized;
		bool _ignoreVoiceTicksInitialized;
		bool _VoIPChannelVolumeInitialized;
	};

	class IgnoreLocalPlayer : public BaseServerCommand
	{
	public:
		IgnoreLocalPlayer() :
			_playerInitialized( false ),
			_ignoreInitialized( false ),
			_doVoiceInitialized( false ),
			_ticksInitialized( false ),
			_reasonInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetIgnore( bool value );
		void SetDoVoice( bool value );
		void SetTicks( int value );
		void SetReason( const FString & value );
		bool CheckIgnore() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _ignoreInitialized
				&& _doVoiceInitialized
				&& _ticksInitialized
				&& _reasonInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _ignoreInitialized == false )
				Printf( "Missing: ignore\n" );
			if ( _doVoiceInitialized == false )
				Printf( "Missing: doVoice\n" );
			if ( _ticksInitialized == false )
				Printf( "Missing: ticks\n" );
			if ( _reasonInitialized == false )
				Printf( "Missing: reason\n" );
		}

	protected:
		player_t *player;
		bool ignore;
		bool doVoice;
		int ticks;
		FString reason;
		bool _playerInitialized;
		bool _ignoreInitialized;
		bool _doVoiceInitialized;
		bool _ticksInitialized;
		bool _reasonInitialized;
	};

	class Print : public BaseServerCommand
	{
	public:
		Print() :
			_printlevelInitialized( false ),
			_messageInitialized( false ) {}
		void SetPrintlevel( int value );
		void SetMessage( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _printlevelInitialized
				&& _messageInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _printlevelInitialized == false )
				Printf( "Missing: printlevel\n" );
			if ( _messageInitialized == false )
				Printf( "Missing: message\n" );
		}

	protected:
		int printlevel;
		FString message;
		bool _printlevelInitialized;
		bool _messageInitialized;
	};

	class PrintMid : public BaseServerCommand
	{
	public:
		PrintMid() :
			_messageInitialized( false ),
			_boldInitialized( false ) {}
		void SetMessage( const FString & value );
		void SetBold( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _messageInitialized
				&& _boldInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _messageInitialized == false )
				Printf( "Missing: message\n" );
			if ( _boldInitialized == false )
				Printf( "Missing: bold\n" );
		}

	protected:
		FString message;
		bool bold;
		bool _messageInitialized;
		bool _boldInitialized;
	};

	class PrintMOTD : public BaseServerCommand
	{
	public:
		PrintMOTD() :
			_motdInitialized( false ) {}
		void SetMotd( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _motdInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _motdInitialized == false )
				Printf( "Missing: motd\n" );
		}

	protected:
		FString motd;
		bool _motdInitialized;
	};

	class PrintHUDMessage : public BaseServerCommand
	{
	public:
		PrintHUDMessage() :
			_messageInitialized( false ),
			_xInitialized( false ),
			_yInitialized( false ),
			_typeInitialized( false ),
			_colorInitialized( false ),
			_holdTimeInitialized( false ),
			_idInitialized( false ),
			_inTimeInitialized( false ),
			_outTimeInitialized( false ),
			_hudWidthInitialized( false ),
			_hudHeightInitialized( false ),
			_clipRectLeftInitialized( false ),
			_clipRectTopInitialized( false ),
			_clipRectWidthInitialized( false ),
			_clipRectHeightInitialized( false ),
			_wrapWidthInitialized( false ),
			_fontNameInitialized( false ),
			_alphaInitialized( false ) {}
		void SetMessage( const FString & value );
		void SetX( float value );
		void SetY( float value );
		void SetType( int value );
		void SetColor( int value );
		void SetHoldTime( float value );
		void SetId( int value );
		void SetInTime( float value );
		void SetOutTime( float value );
		void SetHudWidth( int value );
		void SetHudHeight( int value );
		void SetClipRectLeft( int value );
		void SetClipRectTop( int value );
		void SetClipRectWidth( int value );
		void SetClipRectHeight( int value );
		void SetWrapWidth( int value );
		void SetFontName( const FString & value );
		void SetAlpha( fixed_t value );
		bool ContainsAlpha() const;
		bool ContainsClippingRectangle() const;
		bool ContainsFontName() const;
		bool ContainsHUDSize() const;
		bool ContainsInTime() const;
		bool ContainsOutTime() const;
		bool ContainsWrapWidth() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _messageInitialized
				&& _xInitialized
				&& _yInitialized
				&& _typeInitialized
				&& _colorInitialized
				&& _holdTimeInitialized
				&& _idInitialized
				&& _inTimeInitialized
				&& _outTimeInitialized
				&& _hudWidthInitialized
				&& _hudHeightInitialized
				&& _clipRectLeftInitialized
				&& _clipRectTopInitialized
				&& _clipRectWidthInitialized
				&& _clipRectHeightInitialized
				&& _wrapWidthInitialized
				&& _fontNameInitialized
				&& _alphaInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _messageInitialized == false )
				Printf( "Missing: message\n" );
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _typeInitialized == false )
				Printf( "Missing: type\n" );
			if ( _colorInitialized == false )
				Printf( "Missing: color\n" );
			if ( _holdTimeInitialized == false )
				Printf( "Missing: holdTime\n" );
			if ( _idInitialized == false )
				Printf( "Missing: id\n" );
			if ( _inTimeInitialized == false )
				Printf( "Missing: inTime\n" );
			if ( _outTimeInitialized == false )
				Printf( "Missing: outTime\n" );
			if ( _hudWidthInitialized == false )
				Printf( "Missing: hudWidth\n" );
			if ( _hudHeightInitialized == false )
				Printf( "Missing: hudHeight\n" );
			if ( _clipRectLeftInitialized == false )
				Printf( "Missing: clipRectLeft\n" );
			if ( _clipRectTopInitialized == false )
				Printf( "Missing: clipRectTop\n" );
			if ( _clipRectWidthInitialized == false )
				Printf( "Missing: clipRectWidth\n" );
			if ( _clipRectHeightInitialized == false )
				Printf( "Missing: clipRectHeight\n" );
			if ( _wrapWidthInitialized == false )
				Printf( "Missing: wrapWidth\n" );
			if ( _fontNameInitialized == false )
				Printf( "Missing: fontName\n" );
			if ( _alphaInitialized == false )
				Printf( "Missing: alpha\n" );
		}

	protected:
		FString message;
		float x;
		float y;
		int type;
		int color;
		float holdTime;
		int id;
		float inTime;
		float outTime;
		int hudWidth;
		int hudHeight;
		int clipRectLeft;
		int clipRectTop;
		int clipRectWidth;
		int clipRectHeight;
		int wrapWidth;
		FString fontName;
		fixed_t alpha;
		bool _messageInitialized;
		bool _xInitialized;
		bool _yInitialized;
		bool _typeInitialized;
		bool _colorInitialized;
		bool _holdTimeInitialized;
		bool _idInitialized;
		bool _inTimeInitialized;
		bool _outTimeInitialized;
		bool _hudWidthInitialized;
		bool _hudHeightInitialized;
		bool _clipRectLeftInitialized;
		bool _clipRectTopInitialized;
		bool _clipRectWidthInitialized;
		bool _clipRectHeightInitialized;
		bool _wrapWidthInitialized;
		bool _fontNameInitialized;
		bool _alphaInitialized;
	};

	class PrintTeamScoresMessage : public BaseServerCommand
	{
	public:
		PrintTeamScoresMessage() :
			_teamInitialized( false ),
			_scorerInitialized( false ),
			_assisterInitialized( false ),
			_numPointsInitialized( false ) {}
		void SetTeam( int value );
		void SetScorer( int value );
		void SetAssister( int value );
		void SetNumPoints( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _teamInitialized
				&& _scorerInitialized
				&& _assisterInitialized
				&& _numPointsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _teamInitialized == false )
				Printf( "Missing: team\n" );
			if ( _scorerInitialized == false )
				Printf( "Missing: scorer\n" );
			if ( _assisterInitialized == false )
				Printf( "Missing: assister\n" );
			if ( _numPointsInitialized == false )
				Printf( "Missing: numPoints\n" );
		}

	protected:
		int team;
		int scorer;
		int assister;
		int numPoints;
		bool _teamInitialized;
		bool _scorerInitialized;
		bool _assisterInitialized;
		bool _numPointsInitialized;
	};

	class SpawnThing : public BaseServerCommand
	{
	public:
		SpawnThing() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_typeInitialized( false ),
			_idInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetType( const PClass * value );
		void SetId( unsigned short value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _typeInitialized
				&& _idInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _typeInitialized == false )
				Printf( "Missing: type\n" );
			if ( _idInitialized == false )
				Printf( "Missing: id\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		const PClass *type;
		unsigned short id;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _typeInitialized;
		bool _idInitialized;
	};

	class SpawnThingNoNetID : public BaseServerCommand
	{
	public:
		SpawnThingNoNetID() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_typeInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetType( const PClass * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _typeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _typeInitialized == false )
				Printf( "Missing: type\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		const PClass *type;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _typeInitialized;
	};

	class SpawnThingExact : public BaseServerCommand
	{
	public:
		SpawnThingExact() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_typeInitialized( false ),
			_idInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetType( const PClass * value );
		void SetId( unsigned short value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _typeInitialized
				&& _idInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _typeInitialized == false )
				Printf( "Missing: type\n" );
			if ( _idInitialized == false )
				Printf( "Missing: id\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		const PClass *type;
		unsigned short id;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _typeInitialized;
		bool _idInitialized;
	};

	class SpawnThingExactNoNetID : public BaseServerCommand
	{
	public:
		SpawnThingExactNoNetID() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_typeInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetType( const PClass * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _typeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _typeInitialized == false )
				Printf( "Missing: type\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		const PClass *type;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _typeInitialized;
	};

	class LevelSpawnThing : public BaseServerCommand
	{
	public:
		LevelSpawnThing() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_typeInitialized( false ),
			_idInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetType( const PClass * value );
		void SetId( unsigned short value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _typeInitialized
				&& _idInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _typeInitialized == false )
				Printf( "Missing: type\n" );
			if ( _idInitialized == false )
				Printf( "Missing: id\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		const PClass *type;
		unsigned short id;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _typeInitialized;
		bool _idInitialized;
	};

	class LevelSpawnThingNoNetID : public BaseServerCommand
	{
	public:
		LevelSpawnThingNoNetID() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_typeInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetType( const PClass * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _typeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _typeInitialized == false )
				Printf( "Missing: type\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		const PClass *type;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _typeInitialized;
	};

	class MoveThing : public BaseServerCommand
	{
	public:
		MoveThing() :
			_actorInitialized( false ),
			_bitsInitialized( false ),
			_newXInitialized( false ),
			_newYInitialized( false ),
			_newZInitialized( false ),
			_lastXInitialized( false ),
			_lastYInitialized( false ),
			_lastZInitialized( false ),
			_angleInitialized( false ),
			_velXInitialized( false ),
			_velYInitialized( false ),
			_velZInitialized( false ),
			_pitchInitialized( false ),
			_movedirInitialized( false ) {}
		void SetActor( AActor * value );
		void SetBits( int value );
		void SetNewX( fixed_t value );
		void SetNewY( fixed_t value );
		void SetNewZ( fixed_t value );
		void SetLastX( fixed_t value );
		void SetLastY( fixed_t value );
		void SetLastZ( fixed_t value );
		void SetAngle( angle_t value );
		void SetVelX( fixed_t value );
		void SetVelY( fixed_t value );
		void SetVelZ( fixed_t value );
		void SetPitch( int value );
		void SetMovedir( int value );
		bool ContainsAngle() const;
		bool ContainsLastX() const;
		bool ContainsLastY() const;
		bool ContainsLastZ() const;
		bool ContainsMovedir() const;
		bool ContainsNewX() const;
		bool ContainsNewY() const;
		bool ContainsNewZ() const;
		bool ContainsPitch() const;
		bool ContainsVelX() const;
		bool ContainsVelY() const;
		bool ContainsVelZ() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _bitsInitialized
				&& _newXInitialized
				&& _newYInitialized
				&& _newZInitialized
				&& _lastXInitialized
				&& _lastYInitialized
				&& _lastZInitialized
				&& _angleInitialized
				&& _velXInitialized
				&& _velYInitialized
				&& _velZInitialized
				&& _pitchInitialized
				&& _movedirInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _bitsInitialized == false )
				Printf( "Missing: bits\n" );
			if ( _newXInitialized == false )
				Printf( "Missing: newX\n" );
			if ( _newYInitialized == false )
				Printf( "Missing: newY\n" );
			if ( _newZInitialized == false )
				Printf( "Missing: newZ\n" );
			if ( _lastXInitialized == false )
				Printf( "Missing: lastX\n" );
			if ( _lastYInitialized == false )
				Printf( "Missing: lastY\n" );
			if ( _lastZInitialized == false )
				Printf( "Missing: lastZ\n" );
			if ( _angleInitialized == false )
				Printf( "Missing: angle\n" );
			if ( _velXInitialized == false )
				Printf( "Missing: velX\n" );
			if ( _velYInitialized == false )
				Printf( "Missing: velY\n" );
			if ( _velZInitialized == false )
				Printf( "Missing: velZ\n" );
			if ( _pitchInitialized == false )
				Printf( "Missing: pitch\n" );
			if ( _movedirInitialized == false )
				Printf( "Missing: movedir\n" );
		}

	protected:
		AActor *actor;
		int bits;
		fixed_t newX;
		fixed_t newY;
		fixed_t newZ;
		fixed_t lastX;
		fixed_t lastY;
		fixed_t lastZ;
		angle_t angle;
		fixed_t velX;
		fixed_t velY;
		fixed_t velZ;
		int pitch;
		int movedir;
		bool _actorInitialized;
		bool _bitsInitialized;
		bool _newXInitialized;
		bool _newYInitialized;
		bool _newZInitialized;
		bool _lastXInitialized;
		bool _lastYInitialized;
		bool _lastZInitialized;
		bool _angleInitialized;
		bool _velXInitialized;
		bool _velYInitialized;
		bool _velZInitialized;
		bool _pitchInitialized;
		bool _movedirInitialized;
	};

	class MoveThingExact : public BaseServerCommand
	{
	public:
		MoveThingExact() :
			_actorInitialized( false ),
			_bitsInitialized( false ),
			_newXInitialized( false ),
			_newYInitialized( false ),
			_newZInitialized( false ),
			_lastXInitialized( false ),
			_lastYInitialized( false ),
			_lastZInitialized( false ),
			_angleInitialized( false ),
			_velXInitialized( false ),
			_velYInitialized( false ),
			_velZInitialized( false ),
			_pitchInitialized( false ),
			_movedirInitialized( false ) {}
		void SetActor( AActor * value );
		void SetBits( int value );
		void SetNewX( fixed_t value );
		void SetNewY( fixed_t value );
		void SetNewZ( fixed_t value );
		void SetLastX( fixed_t value );
		void SetLastY( fixed_t value );
		void SetLastZ( fixed_t value );
		void SetAngle( angle_t value );
		void SetVelX( fixed_t value );
		void SetVelY( fixed_t value );
		void SetVelZ( fixed_t value );
		void SetPitch( int value );
		void SetMovedir( int value );
		bool ContainsAngle() const;
		bool ContainsLastX() const;
		bool ContainsLastY() const;
		bool ContainsLastZ() const;
		bool ContainsMovedir() const;
		bool ContainsNewX() const;
		bool ContainsNewY() const;
		bool ContainsNewZ() const;
		bool ContainsPitch() const;
		bool ContainsVelX() const;
		bool ContainsVelY() const;
		bool ContainsVelZ() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _bitsInitialized
				&& _newXInitialized
				&& _newYInitialized
				&& _newZInitialized
				&& _lastXInitialized
				&& _lastYInitialized
				&& _lastZInitialized
				&& _angleInitialized
				&& _velXInitialized
				&& _velYInitialized
				&& _velZInitialized
				&& _pitchInitialized
				&& _movedirInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _bitsInitialized == false )
				Printf( "Missing: bits\n" );
			if ( _newXInitialized == false )
				Printf( "Missing: newX\n" );
			if ( _newYInitialized == false )
				Printf( "Missing: newY\n" );
			if ( _newZInitialized == false )
				Printf( "Missing: newZ\n" );
			if ( _lastXInitialized == false )
				Printf( "Missing: lastX\n" );
			if ( _lastYInitialized == false )
				Printf( "Missing: lastY\n" );
			if ( _lastZInitialized == false )
				Printf( "Missing: lastZ\n" );
			if ( _angleInitialized == false )
				Printf( "Missing: angle\n" );
			if ( _velXInitialized == false )
				Printf( "Missing: velX\n" );
			if ( _velYInitialized == false )
				Printf( "Missing: velY\n" );
			if ( _velZInitialized == false )
				Printf( "Missing: velZ\n" );
			if ( _pitchInitialized == false )
				Printf( "Missing: pitch\n" );
			if ( _movedirInitialized == false )
				Printf( "Missing: movedir\n" );
		}

	protected:
		AActor *actor;
		int bits;
		fixed_t newX;
		fixed_t newY;
		fixed_t newZ;
		fixed_t lastX;
		fixed_t lastY;
		fixed_t lastZ;
		angle_t angle;
		fixed_t velX;
		fixed_t velY;
		fixed_t velZ;
		int pitch;
		int movedir;
		bool _actorInitialized;
		bool _bitsInitialized;
		bool _newXInitialized;
		bool _newYInitialized;
		bool _newZInitialized;
		bool _lastXInitialized;
		bool _lastYInitialized;
		bool _lastZInitialized;
		bool _angleInitialized;
		bool _velXInitialized;
		bool _velYInitialized;
		bool _velZInitialized;
		bool _pitchInitialized;
		bool _movedirInitialized;
	};

	class KillThing : public BaseServerCommand
	{
	public:
		KillThing() :
			_victimInitialized( false ),
			_healthInitialized( false ),
			_damageTypeInitialized( false ),
			_sourceInitialized( false ),
			_inflictorInitialized( false ) {}
		void SetVictim( AActor * value );
		void SetHealth( int value );
		void SetDamageType( const FString & value );
		void SetSource( AActor * value );
		void SetInflictor( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _victimInitialized
				&& _healthInitialized
				&& _damageTypeInitialized
				&& _sourceInitialized
				&& _inflictorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _victimInitialized == false )
				Printf( "Missing: victim\n" );
			if ( _healthInitialized == false )
				Printf( "Missing: health\n" );
			if ( _damageTypeInitialized == false )
				Printf( "Missing: damageType\n" );
			if ( _sourceInitialized == false )
				Printf( "Missing: source\n" );
			if ( _inflictorInitialized == false )
				Printf( "Missing: inflictor\n" );
		}

	protected:
		AActor *victim;
		int health;
		FName damageType;
		AActor *source;
		AActor *inflictor;
		bool _victimInitialized;
		bool _healthInitialized;
		bool _damageTypeInitialized;
		bool _sourceInitialized;
		bool _inflictorInitialized;
	};

	class SetThingState : public BaseServerCommand
	{
	public:
		SetThingState() :
			_actorInitialized( false ),
			_stateInitialized( false ) {}
		void SetActor( AActor * value );
		void SetState( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _stateInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _stateInitialized == false )
				Printf( "Missing: state\n" );
		}

	protected:
		AActor *actor;
		int state;
		bool _actorInitialized;
		bool _stateInitialized;
	};

	class SetThingTarget : public BaseServerCommand
	{
	public:
		SetThingTarget() :
			_actorInitialized( false ),
			_targetInitialized( false ) {}
		void SetActor( AActor * value );
		void SetTarget( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _targetInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _targetInitialized == false )
				Printf( "Missing: target\n" );
		}

	protected:
		AActor *actor;
		AActor *target;
		bool _actorInitialized;
		bool _targetInitialized;
	};

	class DestroyThing : public BaseServerCommand
	{
	public:
		DestroyThing() :
			_actorInitialized( false ) {}
		void SetActor( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
		}

	protected:
		AActor *actor;
		bool _actorInitialized;
	};

	class SetThingAngle : public BaseServerCommand
	{
	public:
		SetThingAngle() :
			_actorInitialized( false ),
			_angleInitialized( false ) {}
		void SetActor( AActor * value );
		void SetAngle( angle_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _angleInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _angleInitialized == false )
				Printf( "Missing: angle\n" );
		}

	protected:
		AActor *actor;
		angle_t angle;
		bool _actorInitialized;
		bool _angleInitialized;
	};

	class SetThingAngleExact : public BaseServerCommand
	{
	public:
		SetThingAngleExact() :
			_actorInitialized( false ),
			_angleInitialized( false ) {}
		void SetActor( AActor * value );
		void SetAngle( angle_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _angleInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _angleInitialized == false )
				Printf( "Missing: angle\n" );
		}

	protected:
		AActor *actor;
		angle_t angle;
		bool _actorInitialized;
		bool _angleInitialized;
	};

	class SetThingWaterLevel : public BaseServerCommand
	{
	public:
		SetThingWaterLevel() :
			_actorInitialized( false ),
			_waterlevelInitialized( false ) {}
		void SetActor( AActor * value );
		void SetWaterlevel( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _waterlevelInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _waterlevelInitialized == false )
				Printf( "Missing: waterlevel\n" );
		}

	protected:
		AActor *actor;
		int waterlevel;
		bool _actorInitialized;
		bool _waterlevelInitialized;
	};

	class SetThingFlags : public BaseServerCommand
	{
	public:
		SetThingFlags() :
			_actorInitialized( false ),
			_flagsetInitialized( false ),
			_flagsInitialized( false ) {}
		void SetActor( AActor * value );
		void SetFlagset( int value );
		void SetFlags( unsigned int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _flagsetInitialized
				&& _flagsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _flagsetInitialized == false )
				Printf( "Missing: flagset\n" );
			if ( _flagsInitialized == false )
				Printf( "Missing: flags\n" );
		}

	protected:
		AActor *actor;
		int flagset;
		unsigned int flags;
		bool _actorInitialized;
		bool _flagsetInitialized;
		bool _flagsInitialized;
	};

	class SetThingArguments : public BaseServerCommand
	{
	public:
		SetThingArguments() :
			_actorInitialized( false ),
			_arg0Initialized( false ),
			_arg1Initialized( false ),
			_arg2Initialized( false ),
			_arg3Initialized( false ),
			_arg4Initialized( false ) {}
		void SetActor( AActor * value );
		void SetArg0( int value );
		void SetArg1( int value );
		void SetArg2( int value );
		void SetArg3( int value );
		void SetArg4( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _arg0Initialized
				&& _arg1Initialized
				&& _arg2Initialized
				&& _arg3Initialized
				&& _arg4Initialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _arg0Initialized == false )
				Printf( "Missing: arg0\n" );
			if ( _arg1Initialized == false )
				Printf( "Missing: arg1\n" );
			if ( _arg2Initialized == false )
				Printf( "Missing: arg2\n" );
			if ( _arg3Initialized == false )
				Printf( "Missing: arg3\n" );
			if ( _arg4Initialized == false )
				Printf( "Missing: arg4\n" );
		}

	protected:
		AActor *actor;
		int arg0;
		int arg1;
		int arg2;
		int arg3;
		int arg4;
		bool _actorInitialized;
		bool _arg0Initialized;
		bool _arg1Initialized;
		bool _arg2Initialized;
		bool _arg3Initialized;
		bool _arg4Initialized;
	};

	class SetThingTranslation : public BaseServerCommand
	{
	public:
		SetThingTranslation() :
			_actorInitialized( false ),
			_translationInitialized( false ) {}
		void SetActor( AActor * value );
		void SetTranslation( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _translationInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _translationInitialized == false )
				Printf( "Missing: translation\n" );
		}

	protected:
		AActor *actor;
		int translation;
		bool _actorInitialized;
		bool _translationInitialized;
	};

	class SetThingProperty : public BaseServerCommand
	{
	public:
		SetThingProperty() :
			_actorInitialized( false ),
			_propertyInitialized( false ),
			_valueInitialized( false ) {}
		void SetActor( AActor * value );
		void SetProperty( int value );
		void SetValue( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _propertyInitialized
				&& _valueInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _propertyInitialized == false )
				Printf( "Missing: property\n" );
			if ( _valueInitialized == false )
				Printf( "Missing: value\n" );
		}

	protected:
		AActor *actor;
		int property;
		int value;
		bool _actorInitialized;
		bool _propertyInitialized;
		bool _valueInitialized;
	};

	class SetThingStringProperty : public BaseServerCommand
	{
	public:
		SetThingStringProperty() :
			_actorInitialized( false ),
			_propertyInitialized( false ),
			_valueInitialized( false ) {}
		void SetActor( AActor * value );
		void SetProperty( int value );
		void SetValue( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _propertyInitialized
				&& _valueInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _propertyInitialized == false )
				Printf( "Missing: property\n" );
			if ( _valueInitialized == false )
				Printf( "Missing: value\n" );
		}

	protected:
		AActor *actor;
		int property;
		FString value;
		bool _actorInitialized;
		bool _propertyInitialized;
		bool _valueInitialized;
	};

	class SetThingSound : public BaseServerCommand
	{
	public:
		SetThingSound() :
			_actorInitialized( false ),
			_soundTypeInitialized( false ),
			_soundInitialized( false ) {}
		void SetActor( AActor * value );
		void SetSoundType( int value );
		void SetSound( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _soundTypeInitialized
				&& _soundInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _soundTypeInitialized == false )
				Printf( "Missing: soundType\n" );
			if ( _soundInitialized == false )
				Printf( "Missing: sound\n" );
		}

	protected:
		AActor *actor;
		int soundType;
		FString sound;
		bool _actorInitialized;
		bool _soundTypeInitialized;
		bool _soundInitialized;
	};

	class SetThingSpawnPoint : public BaseServerCommand
	{
	public:
		SetThingSpawnPoint() :
			_actorInitialized( false ),
			_spawnPointXInitialized( false ),
			_spawnPointYInitialized( false ),
			_spawnPointZInitialized( false ) {}
		void SetActor( AActor * value );
		void SetSpawnPointX( fixed_t value );
		void SetSpawnPointY( fixed_t value );
		void SetSpawnPointZ( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _spawnPointXInitialized
				&& _spawnPointYInitialized
				&& _spawnPointZInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _spawnPointXInitialized == false )
				Printf( "Missing: spawnPointX\n" );
			if ( _spawnPointYInitialized == false )
				Printf( "Missing: spawnPointY\n" );
			if ( _spawnPointZInitialized == false )
				Printf( "Missing: spawnPointZ\n" );
		}

	protected:
		AActor *actor;
		fixed_t spawnPointX;
		fixed_t spawnPointY;
		fixed_t spawnPointZ;
		bool _actorInitialized;
		bool _spawnPointXInitialized;
		bool _spawnPointYInitialized;
		bool _spawnPointZInitialized;
	};

	class SetThingSpecial1 : public BaseServerCommand
	{
	public:
		SetThingSpecial1() :
			_actorInitialized( false ),
			_special1Initialized( false ) {}
		void SetActor( AActor * value );
		void SetSpecial1( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _special1Initialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _special1Initialized == false )
				Printf( "Missing: special1\n" );
		}

	protected:
		AActor *actor;
		int special1;
		bool _actorInitialized;
		bool _special1Initialized;
	};

	class SetThingSpecial2 : public BaseServerCommand
	{
	public:
		SetThingSpecial2() :
			_actorInitialized( false ),
			_special2Initialized( false ) {}
		void SetActor( AActor * value );
		void SetSpecial2( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _special2Initialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _special2Initialized == false )
				Printf( "Missing: special2\n" );
		}

	protected:
		AActor *actor;
		int special2;
		bool _actorInitialized;
		bool _special2Initialized;
	};

	class SetThingTics : public BaseServerCommand
	{
	public:
		SetThingTics() :
			_actorInitialized( false ),
			_ticsInitialized( false ) {}
		void SetActor( AActor * value );
		void SetTics( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _ticsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _ticsInitialized == false )
				Printf( "Missing: tics\n" );
		}

	protected:
		AActor *actor;
		int tics;
		bool _actorInitialized;
		bool _ticsInitialized;
	};

	class SetThingTID : public BaseServerCommand
	{
	public:
		SetThingTID() :
			_actorInitialized( false ),
			_tidInitialized( false ) {}
		void SetActor( AActor * value );
		void SetTid( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _tidInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _tidInitialized == false )
				Printf( "Missing: tid\n" );
		}

	protected:
		AActor *actor;
		int tid;
		bool _actorInitialized;
		bool _tidInitialized;
	};

	class SetThingReactionTime : public BaseServerCommand
	{
	public:
		SetThingReactionTime() :
			_actorInitialized( false ),
			_reactiontimeInitialized( false ) {}
		void SetActor( AActor * value );
		void SetReactiontime( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _reactiontimeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _reactiontimeInitialized == false )
				Printf( "Missing: reactiontime\n" );
		}

	protected:
		AActor *actor;
		int reactiontime;
		bool _actorInitialized;
		bool _reactiontimeInitialized;
	};

	class SetThingGravity : public BaseServerCommand
	{
	public:
		SetThingGravity() :
			_actorInitialized( false ),
			_gravityInitialized( false ) {}
		void SetActor( AActor * value );
		void SetGravity( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _gravityInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _gravityInitialized == false )
				Printf( "Missing: gravity\n" );
		}

	protected:
		AActor *actor;
		int gravity;
		bool _actorInitialized;
		bool _gravityInitialized;
	};

	class SetThingFrame : public BaseServerCommand
	{
	public:
		SetThingFrame() :
			_actorInitialized( false ),
			_stateOwnerInitialized( false ),
			_offsetInitialized( false ) {}
		void SetActor( AActor * value );
		void SetStateOwner( const PClass * value );
		void SetOffset( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _stateOwnerInitialized
				&& _offsetInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _stateOwnerInitialized == false )
				Printf( "Missing: stateOwner\n" );
			if ( _offsetInitialized == false )
				Printf( "Missing: offset\n" );
		}

	protected:
		AActor *actor;
		const PClass *stateOwner;
		int offset;
		bool _actorInitialized;
		bool _stateOwnerInitialized;
		bool _offsetInitialized;
	};

	class SetThingFrameNF : public BaseServerCommand
	{
	public:
		SetThingFrameNF() :
			_actorInitialized( false ),
			_stateOwnerInitialized( false ),
			_offsetInitialized( false ) {}
		void SetActor( AActor * value );
		void SetStateOwner( const PClass * value );
		void SetOffset( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _stateOwnerInitialized
				&& _offsetInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _stateOwnerInitialized == false )
				Printf( "Missing: stateOwner\n" );
			if ( _offsetInitialized == false )
				Printf( "Missing: offset\n" );
		}

	protected:
		AActor *actor;
		const PClass *stateOwner;
		int offset;
		bool _actorInitialized;
		bool _stateOwnerInitialized;
		bool _offsetInitialized;
	};

	class SetWeaponAmmoGive : public BaseServerCommand
	{
	public:
		SetWeaponAmmoGive() :
			_weaponInitialized( false ),
			_ammoGive1Initialized( false ),
			_ammoGive2Initialized( false ) {}
		void SetWeapon( AWeapon * value );
		void SetAmmoGive1( int value );
		void SetAmmoGive2( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _weaponInitialized
				&& _ammoGive1Initialized
				&& _ammoGive2Initialized;
		}
		void PrintMissingParameters() const
		{
			if ( _weaponInitialized == false )
				Printf( "Missing: weapon\n" );
			if ( _ammoGive1Initialized == false )
				Printf( "Missing: ammoGive1\n" );
			if ( _ammoGive2Initialized == false )
				Printf( "Missing: ammoGive2\n" );
		}

	protected:
		AWeapon *weapon;
		int ammoGive1;
		int ammoGive2;
		bool _weaponInitialized;
		bool _ammoGive1Initialized;
		bool _ammoGive2Initialized;
	};

	class SetThingScale : public BaseServerCommand
	{
	public:
		SetThingScale() :
			_actorInitialized( false ),
			_scaleflagsInitialized( false ),
			_scaleXInitialized( false ),
			_scaleYInitialized( false ) {}
		void SetActor( AActor * value );
		void SetScaleflags( int value );
		void SetScaleX( int value );
		void SetScaleY( int value );
		bool ContainsScaleX() const;
		bool ContainsScaleY() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _scaleflagsInitialized
				&& _scaleXInitialized
				&& _scaleYInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _scaleflagsInitialized == false )
				Printf( "Missing: scaleflags\n" );
			if ( _scaleXInitialized == false )
				Printf( "Missing: scaleX\n" );
			if ( _scaleYInitialized == false )
				Printf( "Missing: scaleY\n" );
		}

	protected:
		AActor *actor;
		int scaleflags;
		int scaleX;
		int scaleY;
		bool _actorInitialized;
		bool _scaleflagsInitialized;
		bool _scaleXInitialized;
		bool _scaleYInitialized;
	};

	class ThingIsCorpse : public BaseServerCommand
	{
	public:
		ThingIsCorpse() :
			_actorInitialized( false ),
			_isMonsterInitialized( false ) {}
		void SetActor( AActor * value );
		void SetIsMonster( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _isMonsterInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _isMonsterInitialized == false )
				Printf( "Missing: isMonster\n" );
		}

	protected:
		AActor *actor;
		bool isMonster;
		bool _actorInitialized;
		bool _isMonsterInitialized;
	};

	class HideThing : public BaseServerCommand
	{
	public:
		HideThing() :
			_itemInitialized( false ) {}
		void SetItem( AInventory * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _itemInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _itemInitialized == false )
				Printf( "Missing: item\n" );
		}

	protected:
		AInventory *item;
		bool _itemInitialized;
	};

	class TeleportThing : public BaseServerCommand
	{
	public:
		TeleportThing() :
			_actorInitialized( false ),
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_momxInitialized( false ),
			_momyInitialized( false ),
			_momzInitialized( false ),
			_reactiontimeInitialized( false ),
			_angleInitialized( false ),
			_sourcefogInitialized( false ),
			_destfogInitialized( false ),
			_teleportzoomInitialized( false ) {}
		void SetActor( AActor * value );
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetMomx( fixed_t value );
		void SetMomy( fixed_t value );
		void SetMomz( fixed_t value );
		void SetReactiontime( int value );
		void SetAngle( angle_t value );
		void SetSourcefog( bool value );
		void SetDestfog( bool value );
		void SetTeleportzoom( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _momxInitialized
				&& _momyInitialized
				&& _momzInitialized
				&& _reactiontimeInitialized
				&& _angleInitialized
				&& _sourcefogInitialized
				&& _destfogInitialized
				&& _teleportzoomInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _momxInitialized == false )
				Printf( "Missing: momx\n" );
			if ( _momyInitialized == false )
				Printf( "Missing: momy\n" );
			if ( _momzInitialized == false )
				Printf( "Missing: momz\n" );
			if ( _reactiontimeInitialized == false )
				Printf( "Missing: reactiontime\n" );
			if ( _angleInitialized == false )
				Printf( "Missing: angle\n" );
			if ( _sourcefogInitialized == false )
				Printf( "Missing: sourcefog\n" );
			if ( _destfogInitialized == false )
				Printf( "Missing: destfog\n" );
			if ( _teleportzoomInitialized == false )
				Printf( "Missing: teleportzoom\n" );
		}

	protected:
		AActor *actor;
		fixed_t x;
		fixed_t y;
		fixed_t z;
		fixed_t momx;
		fixed_t momy;
		fixed_t momz;
		int reactiontime;
		angle_t angle;
		bool sourcefog;
		bool destfog;
		bool teleportzoom;
		bool _actorInitialized;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _momxInitialized;
		bool _momyInitialized;
		bool _momzInitialized;
		bool _reactiontimeInitialized;
		bool _angleInitialized;
		bool _sourcefogInitialized;
		bool _destfogInitialized;
		bool _teleportzoomInitialized;
	};

	class ThingActivate : public BaseServerCommand
	{
	public:
		ThingActivate() :
			_actorInitialized( false ),
			_activatorInitialized( false ) {}
		void SetActor( AActor * value );
		void SetActivator( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _activatorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _activatorInitialized == false )
				Printf( "Missing: activator\n" );
		}

	protected:
		AActor *actor;
		AActor *activator;
		bool _actorInitialized;
		bool _activatorInitialized;
	};

	class ThingDeactivate : public BaseServerCommand
	{
	public:
		ThingDeactivate() :
			_actorInitialized( false ),
			_activatorInitialized( false ) {}
		void SetActor( AActor * value );
		void SetActivator( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _activatorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _activatorInitialized == false )
				Printf( "Missing: activator\n" );
		}

	protected:
		AActor *actor;
		AActor *activator;
		bool _actorInitialized;
		bool _activatorInitialized;
	};

	class RespawnDoomThing : public BaseServerCommand
	{
	public:
		RespawnDoomThing() :
			_actorInitialized( false ),
			_fogInitialized( false ) {}
		void SetActor( AActor * value );
		void SetFog( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _fogInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _fogInitialized == false )
				Printf( "Missing: fog\n" );
		}

	protected:
		AActor *actor;
		bool fog;
		bool _actorInitialized;
		bool _fogInitialized;
	};

	class RespawnRavenThing : public BaseServerCommand
	{
	public:
		RespawnRavenThing() :
			_actorInitialized( false ) {}
		void SetActor( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
		}

	protected:
		AActor *actor;
		bool _actorInitialized;
	};

	class SpawnBlood : public BaseServerCommand
	{
	public:
		SpawnBlood() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_dirInitialized( false ),
			_damageInitialized( false ),
			_originatorInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetDir( angle_t value );
		void SetDamage( int value );
		void SetOriginator( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _dirInitialized
				&& _damageInitialized
				&& _originatorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _dirInitialized == false )
				Printf( "Missing: dir\n" );
			if ( _damageInitialized == false )
				Printf( "Missing: damage\n" );
			if ( _originatorInitialized == false )
				Printf( "Missing: originator\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		angle_t dir;
		int damage;
		AActor *originator;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _dirInitialized;
		bool _damageInitialized;
		bool _originatorInitialized;
	};

	class SpawnBloodSplatter : public BaseServerCommand
	{
	public:
		SpawnBloodSplatter() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_originatorInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetOriginator( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _originatorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _originatorInitialized == false )
				Printf( "Missing: originator\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		AActor *originator;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _originatorInitialized;
	};

	class SpawnBloodSplatter2 : public BaseServerCommand
	{
	public:
		SpawnBloodSplatter2() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_originatorInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetOriginator( AActor * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _originatorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _originatorInitialized == false )
				Printf( "Missing: originator\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		AActor *originator;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _originatorInitialized;
	};

	class SpawnPuff : public BaseServerCommand
	{
	public:
		SpawnPuff() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_pufftypeInitialized( false ),
			_idInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetPufftype( const PClass * value );
		void SetId( unsigned short value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _pufftypeInitialized
				&& _idInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _pufftypeInitialized == false )
				Printf( "Missing: pufftype\n" );
			if ( _idInitialized == false )
				Printf( "Missing: id\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		const PClass *pufftype;
		unsigned short id;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _pufftypeInitialized;
		bool _idInitialized;
	};

	class SpawnPuffNoNetID : public BaseServerCommand
	{
	public:
		SpawnPuffNoNetID() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_pufftypeInitialized( false ),
			_stateidInitialized( false ),
			_receiveTranslationInitialized( false ),
			_translationInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetPufftype( const PClass * value );
		void SetStateid( int value );
		void SetReceiveTranslation( bool value );
		void SetTranslation( int value );
		bool ContainsTranslation() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _pufftypeInitialized
				&& _stateidInitialized
				&& _receiveTranslationInitialized
				&& _translationInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _pufftypeInitialized == false )
				Printf( "Missing: pufftype\n" );
			if ( _stateidInitialized == false )
				Printf( "Missing: stateid\n" );
			if ( _receiveTranslationInitialized == false )
				Printf( "Missing: receiveTranslation\n" );
			if ( _translationInitialized == false )
				Printf( "Missing: translation\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		const PClass *pufftype;
		int stateid;
		bool receiveTranslation;
		int translation;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _pufftypeInitialized;
		bool _stateidInitialized;
		bool _receiveTranslationInitialized;
		bool _translationInitialized;
	};

	class StartConversation : public BaseServerCommand
	{
	public:
		StartConversation() :
			_npcInitialized( false ),
			_playerInitialized( false ),
			_nodeInitialized( false ),
			_facetalkerInitialized( false ),
			_saveangleInitialized( false ) {}
		void SetNpc( AActor * value );
		void SetPlayer( player_t * value );
		void SetNode( int value );
		void SetFacetalker( bool value );
		void SetSaveangle( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _npcInitialized
				&& _playerInitialized
				&& _nodeInitialized
				&& _facetalkerInitialized
				&& _saveangleInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _npcInitialized == false )
				Printf( "Missing: npc\n" );
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _nodeInitialized == false )
				Printf( "Missing: node\n" );
			if ( _facetalkerInitialized == false )
				Printf( "Missing: facetalker\n" );
			if ( _saveangleInitialized == false )
				Printf( "Missing: saveangle\n" );
		}

	protected:
		AActor *npc;
		player_t *player;
		int node;
		bool facetalker;
		bool saveangle;
		bool _npcInitialized;
		bool _playerInitialized;
		bool _nodeInitialized;
		bool _facetalkerInitialized;
		bool _saveangleInitialized;
	};

	class ConversationReply : public BaseServerCommand
	{
	public:
		ConversationReply() :
			_playerInitialized( false ),
			_nodeInitialized( false ),
			_replyInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetNode( int value );
		void SetReply( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _nodeInitialized
				&& _replyInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _nodeInitialized == false )
				Printf( "Missing: node\n" );
			if ( _replyInitialized == false )
				Printf( "Missing: reply\n" );
		}

	protected:
		player_t *player;
		int node;
		int reply;
		bool _playerInitialized;
		bool _nodeInitialized;
		bool _replyInitialized;
	};

	class EndConversation : public BaseServerCommand
	{
	public:
		EndConversation() :
			_playerInitialized( false ) {}
		void SetPlayer( player_t * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
		}

	protected:
		player_t *player;
		bool _playerInitialized;
	};

	class SetSectorFloorPlane : public BaseServerCommand
	{
	public:
		SetSectorFloorPlane() :
			_sectorInitialized( false ),
			_heightInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetHeight( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _heightInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _heightInitialized == false )
				Printf( "Missing: height\n" );
		}

	protected:
		sector_t *sector;
		fixed_t height;
		bool _sectorInitialized;
		bool _heightInitialized;
	};

	class SetSectorCeilingPlane : public BaseServerCommand
	{
	public:
		SetSectorCeilingPlane() :
			_sectorInitialized( false ),
			_heightInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetHeight( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _heightInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _heightInitialized == false )
				Printf( "Missing: height\n" );
		}

	protected:
		sector_t *sector;
		fixed_t height;
		bool _sectorInitialized;
		bool _heightInitialized;
	};

	class SetSectorFloorPlaneSlope : public BaseServerCommand
	{
	public:
		SetSectorFloorPlaneSlope() :
			_sectorInitialized( false ),
			_aInitialized( false ),
			_bInitialized( false ),
			_cInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetA( fixed_t value );
		void SetB( fixed_t value );
		void SetC( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _aInitialized
				&& _bInitialized
				&& _cInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _aInitialized == false )
				Printf( "Missing: a\n" );
			if ( _bInitialized == false )
				Printf( "Missing: b\n" );
			if ( _cInitialized == false )
				Printf( "Missing: c\n" );
		}

	protected:
		sector_t *sector;
		fixed_t a;
		fixed_t b;
		fixed_t c;
		bool _sectorInitialized;
		bool _aInitialized;
		bool _bInitialized;
		bool _cInitialized;
	};

	class SetSectorCeilingPlaneSlope : public BaseServerCommand
	{
	public:
		SetSectorCeilingPlaneSlope() :
			_sectorInitialized( false ),
			_aInitialized( false ),
			_bInitialized( false ),
			_cInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetA( fixed_t value );
		void SetB( fixed_t value );
		void SetC( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _aInitialized
				&& _bInitialized
				&& _cInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _aInitialized == false )
				Printf( "Missing: a\n" );
			if ( _bInitialized == false )
				Printf( "Missing: b\n" );
			if ( _cInitialized == false )
				Printf( "Missing: c\n" );
		}

	protected:
		sector_t *sector;
		fixed_t a;
		fixed_t b;
		fixed_t c;
		bool _sectorInitialized;
		bool _aInitialized;
		bool _bInitialized;
		bool _cInitialized;
	};

	class SetSectorLightLevel : public BaseServerCommand
	{
	public:
		SetSectorLightLevel() :
			_sectorInitialized( false ),
			_lightLevelInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetLightLevel( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _lightLevelInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _lightLevelInitialized == false )
				Printf( "Missing: lightLevel\n" );
		}

	protected:
		sector_t *sector;
		int lightLevel;
		bool _sectorInitialized;
		bool _lightLevelInitialized;
	};

	class SetSectorColor : public BaseServerCommand
	{
	public:
		SetSectorColor() :
			_sectorInitialized( false ),
			_redInitialized( false ),
			_greenInitialized( false ),
			_blueInitialized( false ),
			_desaturateInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetRed( int value );
		void SetGreen( int value );
		void SetBlue( int value );
		void SetDesaturate( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _redInitialized
				&& _greenInitialized
				&& _blueInitialized
				&& _desaturateInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _redInitialized == false )
				Printf( "Missing: red\n" );
			if ( _greenInitialized == false )
				Printf( "Missing: green\n" );
			if ( _blueInitialized == false )
				Printf( "Missing: blue\n" );
			if ( _desaturateInitialized == false )
				Printf( "Missing: desaturate\n" );
		}

	protected:
		sector_t *sector;
		int red;
		int green;
		int blue;
		int desaturate;
		bool _sectorInitialized;
		bool _redInitialized;
		bool _greenInitialized;
		bool _blueInitialized;
		bool _desaturateInitialized;
	};

	class SetSectorColorByTag : public BaseServerCommand
	{
	public:
		SetSectorColorByTag() :
			_tagInitialized( false ),
			_redInitialized( false ),
			_greenInitialized( false ),
			_blueInitialized( false ),
			_desaturateInitialized( false ) {}
		void SetTag( int value );
		void SetRed( int value );
		void SetGreen( int value );
		void SetBlue( int value );
		void SetDesaturate( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _tagInitialized
				&& _redInitialized
				&& _greenInitialized
				&& _blueInitialized
				&& _desaturateInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _tagInitialized == false )
				Printf( "Missing: tag\n" );
			if ( _redInitialized == false )
				Printf( "Missing: red\n" );
			if ( _greenInitialized == false )
				Printf( "Missing: green\n" );
			if ( _blueInitialized == false )
				Printf( "Missing: blue\n" );
			if ( _desaturateInitialized == false )
				Printf( "Missing: desaturate\n" );
		}

	protected:
		int tag;
		int red;
		int green;
		int blue;
		int desaturate;
		bool _tagInitialized;
		bool _redInitialized;
		bool _greenInitialized;
		bool _blueInitialized;
		bool _desaturateInitialized;
	};

	class SetSectorFade : public BaseServerCommand
	{
	public:
		SetSectorFade() :
			_sectorInitialized( false ),
			_redInitialized( false ),
			_greenInitialized( false ),
			_blueInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetRed( int value );
		void SetGreen( int value );
		void SetBlue( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _redInitialized
				&& _greenInitialized
				&& _blueInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _redInitialized == false )
				Printf( "Missing: red\n" );
			if ( _greenInitialized == false )
				Printf( "Missing: green\n" );
			if ( _blueInitialized == false )
				Printf( "Missing: blue\n" );
		}

	protected:
		sector_t *sector;
		int red;
		int green;
		int blue;
		bool _sectorInitialized;
		bool _redInitialized;
		bool _greenInitialized;
		bool _blueInitialized;
	};

	class SetSectorFadeByTag : public BaseServerCommand
	{
	public:
		SetSectorFadeByTag() :
			_tagInitialized( false ),
			_redInitialized( false ),
			_greenInitialized( false ),
			_blueInitialized( false ) {}
		void SetTag( int value );
		void SetRed( int value );
		void SetGreen( int value );
		void SetBlue( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _tagInitialized
				&& _redInitialized
				&& _greenInitialized
				&& _blueInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _tagInitialized == false )
				Printf( "Missing: tag\n" );
			if ( _redInitialized == false )
				Printf( "Missing: red\n" );
			if ( _greenInitialized == false )
				Printf( "Missing: green\n" );
			if ( _blueInitialized == false )
				Printf( "Missing: blue\n" );
		}

	protected:
		int tag;
		int red;
		int green;
		int blue;
		bool _tagInitialized;
		bool _redInitialized;
		bool _greenInitialized;
		bool _blueInitialized;
	};

	class SetSectorFlat : public BaseServerCommand
	{
	public:
		SetSectorFlat() :
			_sectorInitialized( false ),
			_ceilingFlatNameInitialized( false ),
			_floorFlatNameInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetCeilingFlatName( const FString & value );
		void SetFloorFlatName( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _ceilingFlatNameInitialized
				&& _floorFlatNameInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _ceilingFlatNameInitialized == false )
				Printf( "Missing: ceilingFlatName\n" );
			if ( _floorFlatNameInitialized == false )
				Printf( "Missing: floorFlatName\n" );
		}

	protected:
		sector_t *sector;
		FString ceilingFlatName;
		FString floorFlatName;
		bool _sectorInitialized;
		bool _ceilingFlatNameInitialized;
		bool _floorFlatNameInitialized;
	};

	class SetSectorPanning : public BaseServerCommand
	{
	public:
		SetSectorPanning() :
			_sectorInitialized( false ),
			_ceilingXOffsetInitialized( false ),
			_ceilingYOffsetInitialized( false ),
			_floorXOffsetInitialized( false ),
			_floorYOffsetInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetCeilingXOffset( fixed_t value );
		void SetCeilingYOffset( fixed_t value );
		void SetFloorXOffset( fixed_t value );
		void SetFloorYOffset( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _ceilingXOffsetInitialized
				&& _ceilingYOffsetInitialized
				&& _floorXOffsetInitialized
				&& _floorYOffsetInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _ceilingXOffsetInitialized == false )
				Printf( "Missing: ceilingXOffset\n" );
			if ( _ceilingYOffsetInitialized == false )
				Printf( "Missing: ceilingYOffset\n" );
			if ( _floorXOffsetInitialized == false )
				Printf( "Missing: floorXOffset\n" );
			if ( _floorYOffsetInitialized == false )
				Printf( "Missing: floorYOffset\n" );
		}

	protected:
		sector_t *sector;
		fixed_t ceilingXOffset;
		fixed_t ceilingYOffset;
		fixed_t floorXOffset;
		fixed_t floorYOffset;
		bool _sectorInitialized;
		bool _ceilingXOffsetInitialized;
		bool _ceilingYOffsetInitialized;
		bool _floorXOffsetInitialized;
		bool _floorYOffsetInitialized;
	};

	class SetSectorRotation : public BaseServerCommand
	{
	public:
		SetSectorRotation() :
			_sectorInitialized( false ),
			_ceilingRotationInitialized( false ),
			_floorRotationInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetCeilingRotation( int value );
		void SetFloorRotation( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _ceilingRotationInitialized
				&& _floorRotationInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _ceilingRotationInitialized == false )
				Printf( "Missing: ceilingRotation\n" );
			if ( _floorRotationInitialized == false )
				Printf( "Missing: floorRotation\n" );
		}

	protected:
		sector_t *sector;
		int ceilingRotation;
		int floorRotation;
		bool _sectorInitialized;
		bool _ceilingRotationInitialized;
		bool _floorRotationInitialized;
	};

	class SetSectorRotationByTag : public BaseServerCommand
	{
	public:
		SetSectorRotationByTag() :
			_tagInitialized( false ),
			_ceilingRotationInitialized( false ),
			_floorRotationInitialized( false ) {}
		void SetTag( int value );
		void SetCeilingRotation( int value );
		void SetFloorRotation( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _tagInitialized
				&& _ceilingRotationInitialized
				&& _floorRotationInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _tagInitialized == false )
				Printf( "Missing: tag\n" );
			if ( _ceilingRotationInitialized == false )
				Printf( "Missing: ceilingRotation\n" );
			if ( _floorRotationInitialized == false )
				Printf( "Missing: floorRotation\n" );
		}

	protected:
		int tag;
		int ceilingRotation;
		int floorRotation;
		bool _tagInitialized;
		bool _ceilingRotationInitialized;
		bool _floorRotationInitialized;
	};

	class SetSectorScale : public BaseServerCommand
	{
	public:
		SetSectorScale() :
			_sectorInitialized( false ),
			_ceilingXScaleInitialized( false ),
			_ceilingYScaleInitialized( false ),
			_floorXScaleInitialized( false ),
			_floorYScaleInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetCeilingXScale( fixed_t value );
		void SetCeilingYScale( fixed_t value );
		void SetFloorXScale( fixed_t value );
		void SetFloorYScale( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _ceilingXScaleInitialized
				&& _ceilingYScaleInitialized
				&& _floorXScaleInitialized
				&& _floorYScaleInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _ceilingXScaleInitialized == false )
				Printf( "Missing: ceilingXScale\n" );
			if ( _ceilingYScaleInitialized == false )
				Printf( "Missing: ceilingYScale\n" );
			if ( _floorXScaleInitialized == false )
				Printf( "Missing: floorXScale\n" );
			if ( _floorYScaleInitialized == false )
				Printf( "Missing: floorYScale\n" );
		}

	protected:
		sector_t *sector;
		fixed_t ceilingXScale;
		fixed_t ceilingYScale;
		fixed_t floorXScale;
		fixed_t floorYScale;
		bool _sectorInitialized;
		bool _ceilingXScaleInitialized;
		bool _ceilingYScaleInitialized;
		bool _floorXScaleInitialized;
		bool _floorYScaleInitialized;
	};

	class SetSectorSpecial : public BaseServerCommand
	{
	public:
		SetSectorSpecial() :
			_sectorInitialized( false ),
			_specialInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetSpecial( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _specialInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _specialInitialized == false )
				Printf( "Missing: special\n" );
		}

	protected:
		sector_t *sector;
		int special;
		bool _sectorInitialized;
		bool _specialInitialized;
	};

	class SetSectorFriction : public BaseServerCommand
	{
	public:
		SetSectorFriction() :
			_sectorInitialized( false ),
			_frictionInitialized( false ),
			_moveFactorInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetFriction( fixed_t value );
		void SetMoveFactor( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _frictionInitialized
				&& _moveFactorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _frictionInitialized == false )
				Printf( "Missing: friction\n" );
			if ( _moveFactorInitialized == false )
				Printf( "Missing: moveFactor\n" );
		}

	protected:
		sector_t *sector;
		fixed_t friction;
		fixed_t moveFactor;
		bool _sectorInitialized;
		bool _frictionInitialized;
		bool _moveFactorInitialized;
	};

	class SetSectorAngleYOffset : public BaseServerCommand
	{
	public:
		SetSectorAngleYOffset() :
			_sectorInitialized( false ),
			_ceilingBaseAngleInitialized( false ),
			_ceilingBaseYOffsetInitialized( false ),
			_floorBaseAngleInitialized( false ),
			_floorBaseYOffsetInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetCeilingBaseAngle( fixed_t value );
		void SetCeilingBaseYOffset( fixed_t value );
		void SetFloorBaseAngle( fixed_t value );
		void SetFloorBaseYOffset( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _ceilingBaseAngleInitialized
				&& _ceilingBaseYOffsetInitialized
				&& _floorBaseAngleInitialized
				&& _floorBaseYOffsetInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _ceilingBaseAngleInitialized == false )
				Printf( "Missing: ceilingBaseAngle\n" );
			if ( _ceilingBaseYOffsetInitialized == false )
				Printf( "Missing: ceilingBaseYOffset\n" );
			if ( _floorBaseAngleInitialized == false )
				Printf( "Missing: floorBaseAngle\n" );
			if ( _floorBaseYOffsetInitialized == false )
				Printf( "Missing: floorBaseYOffset\n" );
		}

	protected:
		sector_t *sector;
		fixed_t ceilingBaseAngle;
		fixed_t ceilingBaseYOffset;
		fixed_t floorBaseAngle;
		fixed_t floorBaseYOffset;
		bool _sectorInitialized;
		bool _ceilingBaseAngleInitialized;
		bool _ceilingBaseYOffsetInitialized;
		bool _floorBaseAngleInitialized;
		bool _floorBaseYOffsetInitialized;
	};

	class SetSectorGravity : public BaseServerCommand
	{
	public:
		SetSectorGravity() :
			_sectorInitialized( false ),
			_gravityInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetGravity( float value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _gravityInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _gravityInitialized == false )
				Printf( "Missing: gravity\n" );
		}

	protected:
		sector_t *sector;
		float gravity;
		bool _sectorInitialized;
		bool _gravityInitialized;
	};

	class SetSectorReflection : public BaseServerCommand
	{
	public:
		SetSectorReflection() :
			_sectorInitialized( false ),
			_ceilingReflectionInitialized( false ),
			_floorReflectionInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetCeilingReflection( float value );
		void SetFloorReflection( float value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _ceilingReflectionInitialized
				&& _floorReflectionInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _ceilingReflectionInitialized == false )
				Printf( "Missing: ceilingReflection\n" );
			if ( _floorReflectionInitialized == false )
				Printf( "Missing: floorReflection\n" );
		}

	protected:
		sector_t *sector;
		float ceilingReflection;
		float floorReflection;
		bool _sectorInitialized;
		bool _ceilingReflectionInitialized;
		bool _floorReflectionInitialized;
	};

	class SetSectorLink : public BaseServerCommand
	{
	public:
		SetSectorLink() :
			_sectorInitialized( false ),
			_tagInitialized( false ),
			_ceilingInitialized( false ),
			_moveTypeInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetTag( int value );
		void SetCeiling( int value );
		void SetMoveType( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _tagInitialized
				&& _ceilingInitialized
				&& _moveTypeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _tagInitialized == false )
				Printf( "Missing: tag\n" );
			if ( _ceilingInitialized == false )
				Printf( "Missing: ceiling\n" );
			if ( _moveTypeInitialized == false )
				Printf( "Missing: moveType\n" );
		}

	protected:
		sector_t *sector;
		int tag;
		int ceiling;
		int moveType;
		bool _sectorInitialized;
		bool _tagInitialized;
		bool _ceilingInitialized;
		bool _moveTypeInitialized;
	};

	class StopSectorLightEffect : public BaseServerCommand
	{
	public:
		StopSectorLightEffect() :
			_sectorInitialized( false ) {}
		void SetSector( sector_t * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
		}

	protected:
		sector_t *sector;
		bool _sectorInitialized;
	};

	class DestroyAllSectorMovers : public BaseServerCommand
	{
	public:
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return true;
		}
		void PrintMissingParameters() const
		{
		}

	protected:
	};

	class StartSectorSequence : public BaseServerCommand
	{
	public:
		StartSectorSequence() :
			_sectorInitialized( false ),
			_channelInitialized( false ),
			_sequenceInitialized( false ),
			_modeNumInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetChannel( int value );
		void SetSequence( const FString & value );
		void SetModeNum( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _channelInitialized
				&& _sequenceInitialized
				&& _modeNumInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _channelInitialized == false )
				Printf( "Missing: channel\n" );
			if ( _sequenceInitialized == false )
				Printf( "Missing: sequence\n" );
			if ( _modeNumInitialized == false )
				Printf( "Missing: modeNum\n" );
		}

	protected:
		sector_t *sector;
		int channel;
		FString sequence;
		int modeNum;
		bool _sectorInitialized;
		bool _channelInitialized;
		bool _sequenceInitialized;
		bool _modeNumInitialized;
	};

	class StopSectorSequence : public BaseServerCommand
	{
	public:
		StopSectorSequence() :
			_sectorInitialized( false ) {}
		void SetSector( sector_t * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
		}

	protected:
		sector_t *sector;
		bool _sectorInitialized;
	};

	class DoSectorLightFireFlicker : public BaseServerCommand
	{
	public:
		DoSectorLightFireFlicker() :
			_sectorInitialized( false ),
			_maxLightInitialized( false ),
			_minLightInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetMaxLight( int value );
		void SetMinLight( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _maxLightInitialized
				&& _minLightInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _maxLightInitialized == false )
				Printf( "Missing: maxLight\n" );
			if ( _minLightInitialized == false )
				Printf( "Missing: minLight\n" );
		}

	protected:
		sector_t *sector;
		int maxLight;
		int minLight;
		bool _sectorInitialized;
		bool _maxLightInitialized;
		bool _minLightInitialized;
	};

	class DoSectorLightFlicker : public BaseServerCommand
	{
	public:
		DoSectorLightFlicker() :
			_sectorInitialized( false ),
			_maxLightInitialized( false ),
			_minLightInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetMaxLight( int value );
		void SetMinLight( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _maxLightInitialized
				&& _minLightInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _maxLightInitialized == false )
				Printf( "Missing: maxLight\n" );
			if ( _minLightInitialized == false )
				Printf( "Missing: minLight\n" );
		}

	protected:
		sector_t *sector;
		int maxLight;
		int minLight;
		bool _sectorInitialized;
		bool _maxLightInitialized;
		bool _minLightInitialized;
	};

	class DoSectorLightLightFlash : public BaseServerCommand
	{
	public:
		DoSectorLightLightFlash() :
			_sectorInitialized( false ),
			_maxLightInitialized( false ),
			_minLightInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetMaxLight( int value );
		void SetMinLight( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _maxLightInitialized
				&& _minLightInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _maxLightInitialized == false )
				Printf( "Missing: maxLight\n" );
			if ( _minLightInitialized == false )
				Printf( "Missing: minLight\n" );
		}

	protected:
		sector_t *sector;
		int maxLight;
		int minLight;
		bool _sectorInitialized;
		bool _maxLightInitialized;
		bool _minLightInitialized;
	};

	class DoSectorLightStrobe : public BaseServerCommand
	{
	public:
		DoSectorLightStrobe() :
			_sectorInitialized( false ),
			_darkTimeInitialized( false ),
			_brightTimeInitialized( false ),
			_maxLightInitialized( false ),
			_minLightInitialized( false ),
			_countInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetDarkTime( int value );
		void SetBrightTime( int value );
		void SetMaxLight( int value );
		void SetMinLight( int value );
		void SetCount( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _darkTimeInitialized
				&& _brightTimeInitialized
				&& _maxLightInitialized
				&& _minLightInitialized
				&& _countInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _darkTimeInitialized == false )
				Printf( "Missing: darkTime\n" );
			if ( _brightTimeInitialized == false )
				Printf( "Missing: brightTime\n" );
			if ( _maxLightInitialized == false )
				Printf( "Missing: maxLight\n" );
			if ( _minLightInitialized == false )
				Printf( "Missing: minLight\n" );
			if ( _countInitialized == false )
				Printf( "Missing: count\n" );
		}

	protected:
		sector_t *sector;
		int darkTime;
		int brightTime;
		int maxLight;
		int minLight;
		int count;
		bool _sectorInitialized;
		bool _darkTimeInitialized;
		bool _brightTimeInitialized;
		bool _maxLightInitialized;
		bool _minLightInitialized;
		bool _countInitialized;
	};

	class DoSectorLightGlow : public BaseServerCommand
	{
	public:
		DoSectorLightGlow() :
			_sectorInitialized( false ) {}
		void SetSector( sector_t * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
		}

	protected:
		sector_t *sector;
		bool _sectorInitialized;
	};

	class DoSectorLightGlow2 : public BaseServerCommand
	{
	public:
		DoSectorLightGlow2() :
			_sectorInitialized( false ),
			_startLightInitialized( false ),
			_endLightInitialized( false ),
			_ticsInitialized( false ),
			_maxTicsInitialized( false ),
			_oneShotInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetStartLight( int value );
		void SetEndLight( int value );
		void SetTics( int value );
		void SetMaxTics( int value );
		void SetOneShot( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _startLightInitialized
				&& _endLightInitialized
				&& _ticsInitialized
				&& _maxTicsInitialized
				&& _oneShotInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _startLightInitialized == false )
				Printf( "Missing: startLight\n" );
			if ( _endLightInitialized == false )
				Printf( "Missing: endLight\n" );
			if ( _ticsInitialized == false )
				Printf( "Missing: tics\n" );
			if ( _maxTicsInitialized == false )
				Printf( "Missing: maxTics\n" );
			if ( _oneShotInitialized == false )
				Printf( "Missing: oneShot\n" );
		}

	protected:
		sector_t *sector;
		int startLight;
		int endLight;
		int tics;
		int maxTics;
		bool oneShot;
		bool _sectorInitialized;
		bool _startLightInitialized;
		bool _endLightInitialized;
		bool _ticsInitialized;
		bool _maxTicsInitialized;
		bool _oneShotInitialized;
	};

	class DoSectorLightPhased : public BaseServerCommand
	{
	public:
		DoSectorLightPhased() :
			_sectorInitialized( false ),
			_baseLevelInitialized( false ),
			_phaseInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetBaseLevel( int value );
		void SetPhase( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _baseLevelInitialized
				&& _phaseInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _baseLevelInitialized == false )
				Printf( "Missing: baseLevel\n" );
			if ( _phaseInitialized == false )
				Printf( "Missing: phase\n" );
		}

	protected:
		sector_t *sector;
		int baseLevel;
		int phase;
		bool _sectorInitialized;
		bool _baseLevelInitialized;
		bool _phaseInitialized;
	};

	class SetLineAlpha : public BaseServerCommand
	{
	public:
		SetLineAlpha() :
			_lineInitialized( false ),
			_alphaInitialized( false ) {}
		void SetLine( line_t * value );
		void SetAlpha( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _lineInitialized
				&& _alphaInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _lineInitialized == false )
				Printf( "Missing: line\n" );
			if ( _alphaInitialized == false )
				Printf( "Missing: alpha\n" );
		}

	protected:
		line_t *line;
		fixed_t alpha;
		bool _lineInitialized;
		bool _alphaInitialized;
	};

	class SetLineTexture : public BaseServerCommand
	{
	public:
		SetLineTexture() :
			_lineInitialized( false ),
			_textureNameInitialized( false ),
			_sideInitialized( false ),
			_positionInitialized( false ) {}
		void SetLine( line_t * value );
		void SetTextureName( const FString & value );
		void SetSide( bool value );
		void SetPosition( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _lineInitialized
				&& _textureNameInitialized
				&& _sideInitialized
				&& _positionInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _lineInitialized == false )
				Printf( "Missing: line\n" );
			if ( _textureNameInitialized == false )
				Printf( "Missing: textureName\n" );
			if ( _sideInitialized == false )
				Printf( "Missing: side\n" );
			if ( _positionInitialized == false )
				Printf( "Missing: position\n" );
		}

	protected:
		line_t *line;
		FString textureName;
		bool side;
		int position;
		bool _lineInitialized;
		bool _textureNameInitialized;
		bool _sideInitialized;
		bool _positionInitialized;
	};

	class SetLineTextureByID : public BaseServerCommand
	{
	public:
		SetLineTextureByID() :
			_lineIDInitialized( false ),
			_textureNameInitialized( false ),
			_sideInitialized( false ),
			_positionInitialized( false ) {}
		void SetLineID( unsigned int value );
		void SetTextureName( const FString & value );
		void SetSide( bool value );
		void SetPosition( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _lineIDInitialized
				&& _textureNameInitialized
				&& _sideInitialized
				&& _positionInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _lineIDInitialized == false )
				Printf( "Missing: lineID\n" );
			if ( _textureNameInitialized == false )
				Printf( "Missing: textureName\n" );
			if ( _sideInitialized == false )
				Printf( "Missing: side\n" );
			if ( _positionInitialized == false )
				Printf( "Missing: position\n" );
		}

	protected:
		unsigned int lineID;
		FString textureName;
		bool side;
		int position;
		bool _lineIDInitialized;
		bool _textureNameInitialized;
		bool _sideInitialized;
		bool _positionInitialized;
	};

	class SetLineTextureOffset : public BaseServerCommand
	{
	public:
		SetLineTextureOffset() :
			_lineInitialized( false ),
			_XOffsetInitialized( false ),
			_YOffsetInitialized( false ),
			_sideInitialized( false ),
			_positionInitialized( false ) {}
		void SetLine( line_t * value );
		void SetXOffset( fixed_t value );
		void SetYOffset( fixed_t value );
		void SetSide( bool value );
		void SetPosition( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _lineInitialized
				&& _XOffsetInitialized
				&& _YOffsetInitialized
				&& _sideInitialized
				&& _positionInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _lineInitialized == false )
				Printf( "Missing: line\n" );
			if ( _XOffsetInitialized == false )
				Printf( "Missing: XOffset\n" );
			if ( _YOffsetInitialized == false )
				Printf( "Missing: YOffset\n" );
			if ( _sideInitialized == false )
				Printf( "Missing: side\n" );
			if ( _positionInitialized == false )
				Printf( "Missing: position\n" );
		}

	protected:
		line_t *line;
		fixed_t XOffset;
		fixed_t YOffset;
		bool side;
		int position;
		bool _lineInitialized;
		bool _XOffsetInitialized;
		bool _YOffsetInitialized;
		bool _sideInitialized;
		bool _positionInitialized;
	};

	class SetLineTextureOffsetByID : public BaseServerCommand
	{
	public:
		SetLineTextureOffsetByID() :
			_lineIDInitialized( false ),
			_XOffsetInitialized( false ),
			_YOffsetInitialized( false ),
			_sideInitialized( false ),
			_flagsInitialized( false ) {}
		void SetLineID( unsigned int value );
		void SetXOffset( fixed_t value );
		void SetYOffset( fixed_t value );
		void SetSide( bool value );
		void SetFlags( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _lineIDInitialized
				&& _XOffsetInitialized
				&& _YOffsetInitialized
				&& _sideInitialized
				&& _flagsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _lineIDInitialized == false )
				Printf( "Missing: lineID\n" );
			if ( _XOffsetInitialized == false )
				Printf( "Missing: XOffset\n" );
			if ( _YOffsetInitialized == false )
				Printf( "Missing: YOffset\n" );
			if ( _sideInitialized == false )
				Printf( "Missing: side\n" );
			if ( _flagsInitialized == false )
				Printf( "Missing: flags\n" );
		}

	protected:
		unsigned int lineID;
		fixed_t XOffset;
		fixed_t YOffset;
		bool side;
		int flags;
		bool _lineIDInitialized;
		bool _XOffsetInitialized;
		bool _YOffsetInitialized;
		bool _sideInitialized;
		bool _flagsInitialized;
	};

	class SetLineTextureScale : public BaseServerCommand
	{
	public:
		SetLineTextureScale() :
			_lineInitialized( false ),
			_XScaleInitialized( false ),
			_YScaleInitialized( false ),
			_sideInitialized( false ),
			_positionInitialized( false ) {}
		void SetLine( line_t * value );
		void SetXScale( fixed_t value );
		void SetYScale( fixed_t value );
		void SetSide( bool value );
		void SetPosition( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _lineInitialized
				&& _XScaleInitialized
				&& _YScaleInitialized
				&& _sideInitialized
				&& _positionInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _lineInitialized == false )
				Printf( "Missing: line\n" );
			if ( _XScaleInitialized == false )
				Printf( "Missing: XScale\n" );
			if ( _YScaleInitialized == false )
				Printf( "Missing: YScale\n" );
			if ( _sideInitialized == false )
				Printf( "Missing: side\n" );
			if ( _positionInitialized == false )
				Printf( "Missing: position\n" );
		}

	protected:
		line_t *line;
		fixed_t XScale;
		fixed_t YScale;
		bool side;
		int position;
		bool _lineInitialized;
		bool _XScaleInitialized;
		bool _YScaleInitialized;
		bool _sideInitialized;
		bool _positionInitialized;
	};

	class SetLineTextureScaleByID : public BaseServerCommand
	{
	public:
		SetLineTextureScaleByID() :
			_lineIDInitialized( false ),
			_XScaleInitialized( false ),
			_YScaleInitialized( false ),
			_sideInitialized( false ),
			_flagsInitialized( false ) {}
		void SetLineID( unsigned int value );
		void SetXScale( fixed_t value );
		void SetYScale( fixed_t value );
		void SetSide( bool value );
		void SetFlags( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _lineIDInitialized
				&& _XScaleInitialized
				&& _YScaleInitialized
				&& _sideInitialized
				&& _flagsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _lineIDInitialized == false )
				Printf( "Missing: lineID\n" );
			if ( _XScaleInitialized == false )
				Printf( "Missing: XScale\n" );
			if ( _YScaleInitialized == false )
				Printf( "Missing: YScale\n" );
			if ( _sideInitialized == false )
				Printf( "Missing: side\n" );
			if ( _flagsInitialized == false )
				Printf( "Missing: flags\n" );
		}

	protected:
		unsigned int lineID;
		fixed_t XScale;
		fixed_t YScale;
		bool side;
		int flags;
		bool _lineIDInitialized;
		bool _XScaleInitialized;
		bool _YScaleInitialized;
		bool _sideInitialized;
		bool _flagsInitialized;
	};

	class SetSomeLineFlags : public BaseServerCommand
	{
	public:
		SetSomeLineFlags() :
			_lineInitialized( false ),
			_blockFlagsInitialized( false ) {}
		void SetLine( line_t * value );
		void SetBlockFlags( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _lineInitialized
				&& _blockFlagsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _lineInitialized == false )
				Printf( "Missing: line\n" );
			if ( _blockFlagsInitialized == false )
				Printf( "Missing: blockFlags\n" );
		}

	protected:
		line_t *line;
		int blockFlags;
		bool _lineInitialized;
		bool _blockFlagsInitialized;
	};

	class SetSideFlags : public BaseServerCommand
	{
	public:
		SetSideFlags() :
			_sideInitialized( false ),
			_flagsInitialized( false ) {}
		void SetSide( side_t * value );
		void SetFlags( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sideInitialized
				&& _flagsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sideInitialized == false )
				Printf( "Missing: side\n" );
			if ( _flagsInitialized == false )
				Printf( "Missing: flags\n" );
		}

	protected:
		side_t *side;
		int flags;
		bool _sideInitialized;
		bool _flagsInitialized;
	};

	class Sound : public BaseServerCommand
	{
	public:
		Sound() :
			_channelInitialized( false ),
			_soundInitialized( false ),
			_volumeInitialized( false ),
			_attenuationInitialized( false ) {}
		void SetChannel( int value );
		void SetSound( const FString & value );
		void SetVolume( int value );
		void SetAttenuation( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _channelInitialized
				&& _soundInitialized
				&& _volumeInitialized
				&& _attenuationInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _channelInitialized == false )
				Printf( "Missing: channel\n" );
			if ( _soundInitialized == false )
				Printf( "Missing: sound\n" );
			if ( _volumeInitialized == false )
				Printf( "Missing: volume\n" );
			if ( _attenuationInitialized == false )
				Printf( "Missing: attenuation\n" );
		}

	protected:
		int channel;
		FString sound;
		int volume;
		int attenuation;
		bool _channelInitialized;
		bool _soundInitialized;
		bool _volumeInitialized;
		bool _attenuationInitialized;
	};

	class SoundActor : public BaseServerCommand
	{
	public:
		SoundActor() :
			_actorInitialized( false ),
			_channelInitialized( false ),
			_soundInitialized( false ),
			_volumeInitialized( false ),
			_attenuationInitialized( false ) {}
		void SetActor( AActor * value );
		void SetChannel( int value );
		void SetSound( const FString & value );
		void SetVolume( int value );
		void SetAttenuation( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _channelInitialized
				&& _soundInitialized
				&& _volumeInitialized
				&& _attenuationInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _channelInitialized == false )
				Printf( "Missing: channel\n" );
			if ( _soundInitialized == false )
				Printf( "Missing: sound\n" );
			if ( _volumeInitialized == false )
				Printf( "Missing: volume\n" );
			if ( _attenuationInitialized == false )
				Printf( "Missing: attenuation\n" );
		}

	protected:
		AActor *actor;
		int channel;
		FString sound;
		int volume;
		int attenuation;
		bool _actorInitialized;
		bool _channelInitialized;
		bool _soundInitialized;
		bool _volumeInitialized;
		bool _attenuationInitialized;
	};

	class SoundActorIfNotPlaying : public SoundActor
	{
	public:
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return SoundActor::AllParametersInitialized();
		}
		void PrintMissingParameters() const
		{
			SoundActor::PrintMissingParameters();
		}

	protected:
	};

	class SoundSector : public BaseServerCommand
	{
	public:
		SoundSector() :
			_sectorInitialized( false ),
			_channelInitialized( false ),
			_soundInitialized( false ),
			_volumeInitialized( false ),
			_attenuationInitialized( false ) {}
		void SetSector( sector_t * value );
		void SetChannel( int value );
		void SetSound( const FString & value );
		void SetVolume( int value );
		void SetAttenuation( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sectorInitialized
				&& _channelInitialized
				&& _soundInitialized
				&& _volumeInitialized
				&& _attenuationInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sectorInitialized == false )
				Printf( "Missing: sector\n" );
			if ( _channelInitialized == false )
				Printf( "Missing: channel\n" );
			if ( _soundInitialized == false )
				Printf( "Missing: sound\n" );
			if ( _volumeInitialized == false )
				Printf( "Missing: volume\n" );
			if ( _attenuationInitialized == false )
				Printf( "Missing: attenuation\n" );
		}

	protected:
		sector_t *sector;
		int channel;
		FString sound;
		int volume;
		int attenuation;
		bool _sectorInitialized;
		bool _channelInitialized;
		bool _soundInitialized;
		bool _volumeInitialized;
		bool _attenuationInitialized;
	};

	class SoundPoint : public BaseServerCommand
	{
	public:
		SoundPoint() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_channelInitialized( false ),
			_soundInitialized( false ),
			_volumeInitialized( false ),
			_attenuationInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetChannel( int value );
		void SetSound( const FString & value );
		void SetVolume( int value );
		void SetAttenuation( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _channelInitialized
				&& _soundInitialized
				&& _volumeInitialized
				&& _attenuationInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _channelInitialized == false )
				Printf( "Missing: channel\n" );
			if ( _soundInitialized == false )
				Printf( "Missing: sound\n" );
			if ( _volumeInitialized == false )
				Printf( "Missing: volume\n" );
			if ( _attenuationInitialized == false )
				Printf( "Missing: attenuation\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		int channel;
		FString sound;
		int volume;
		int attenuation;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _channelInitialized;
		bool _soundInitialized;
		bool _volumeInitialized;
		bool _attenuationInitialized;
	};

	class AnnouncerSound : public BaseServerCommand
	{
	public:
		AnnouncerSound() :
			_soundInitialized( false ) {}
		void SetSound( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _soundInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _soundInitialized == false )
				Printf( "Missing: sound\n" );
		}

	protected:
		FString sound;
		bool _soundInitialized;
	};

	class StopSound : public BaseServerCommand
	{
	public:
		StopSound() :
			_actorInitialized( false ),
			_channelInitialized( false ) {}
		void SetActor( AActor * value );
		void SetChannel( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _actorInitialized
				&& _channelInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _actorInitialized == false )
				Printf( "Missing: actor\n" );
			if ( _channelInitialized == false )
				Printf( "Missing: channel\n" );
		}

	protected:
		AActor *actor;
		int channel;
		bool _actorInitialized;
		bool _channelInitialized;
	};

	class StopOriginlessSound : public BaseServerCommand
	{
	public:
		StopOriginlessSound() :
			_channelInitialized( false ) {}
		void SetChannel( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _channelInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _channelInitialized == false )
				Printf( "Missing: channel\n" );
		}

	protected:
		int channel;
		bool _channelInitialized;
	};

	class SpawnMissile : public BaseServerCommand
	{
	public:
		SpawnMissile() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_velXInitialized( false ),
			_velYInitialized( false ),
			_velZInitialized( false ),
			_missileTypeInitialized( false ),
			_netIDInitialized( false ),
			_targetNetIDInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetVelX( fixed_t value );
		void SetVelY( fixed_t value );
		void SetVelZ( fixed_t value );
		void SetMissileType( const PClass * value );
		void SetNetID( unsigned short value );
		void SetTargetNetID( unsigned short value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _velXInitialized
				&& _velYInitialized
				&& _velZInitialized
				&& _missileTypeInitialized
				&& _netIDInitialized
				&& _targetNetIDInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _velXInitialized == false )
				Printf( "Missing: velX\n" );
			if ( _velYInitialized == false )
				Printf( "Missing: velY\n" );
			if ( _velZInitialized == false )
				Printf( "Missing: velZ\n" );
			if ( _missileTypeInitialized == false )
				Printf( "Missing: missileType\n" );
			if ( _netIDInitialized == false )
				Printf( "Missing: netID\n" );
			if ( _targetNetIDInitialized == false )
				Printf( "Missing: targetNetID\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		fixed_t velX;
		fixed_t velY;
		fixed_t velZ;
		const PClass *missileType;
		unsigned short netID;
		unsigned short targetNetID;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _velXInitialized;
		bool _velYInitialized;
		bool _velZInitialized;
		bool _missileTypeInitialized;
		bool _netIDInitialized;
		bool _targetNetIDInitialized;
	};

	class SpawnMissileExact : public BaseServerCommand
	{
	public:
		SpawnMissileExact() :
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ),
			_velXInitialized( false ),
			_velYInitialized( false ),
			_velZInitialized( false ),
			_missileTypeInitialized( false ),
			_netIDInitialized( false ),
			_targetNetIDInitialized( false ) {}
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void SetVelX( fixed_t value );
		void SetVelY( fixed_t value );
		void SetVelZ( fixed_t value );
		void SetMissileType( const PClass * value );
		void SetNetID( unsigned short value );
		void SetTargetNetID( unsigned short value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _xInitialized
				&& _yInitialized
				&& _zInitialized
				&& _velXInitialized
				&& _velYInitialized
				&& _velZInitialized
				&& _missileTypeInitialized
				&& _netIDInitialized
				&& _targetNetIDInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
			if ( _velXInitialized == false )
				Printf( "Missing: velX\n" );
			if ( _velYInitialized == false )
				Printf( "Missing: velY\n" );
			if ( _velZInitialized == false )
				Printf( "Missing: velZ\n" );
			if ( _missileTypeInitialized == false )
				Printf( "Missing: missileType\n" );
			if ( _netIDInitialized == false )
				Printf( "Missing: netID\n" );
			if ( _targetNetIDInitialized == false )
				Printf( "Missing: targetNetID\n" );
		}

	protected:
		fixed_t x;
		fixed_t y;
		fixed_t z;
		fixed_t velX;
		fixed_t velY;
		fixed_t velZ;
		const PClass *missileType;
		unsigned short netID;
		unsigned short targetNetID;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
		bool _velXInitialized;
		bool _velYInitialized;
		bool _velZInitialized;
		bool _missileTypeInitialized;
		bool _netIDInitialized;
		bool _targetNetIDInitialized;
	};

	class MissileExplode : public BaseServerCommand
	{
	public:
		MissileExplode() :
			_missileInitialized( false ),
			_lineIdInitialized( false ),
			_xInitialized( false ),
			_yInitialized( false ),
			_zInitialized( false ) {}
		void SetMissile( AActor * value );
		void SetLineId( int value );
		void SetX( fixed_t value );
		void SetY( fixed_t value );
		void SetZ( fixed_t value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _missileInitialized
				&& _lineIdInitialized
				&& _xInitialized
				&& _yInitialized
				&& _zInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _missileInitialized == false )
				Printf( "Missing: missile\n" );
			if ( _lineIdInitialized == false )
				Printf( "Missing: lineId\n" );
			if ( _xInitialized == false )
				Printf( "Missing: x\n" );
			if ( _yInitialized == false )
				Printf( "Missing: y\n" );
			if ( _zInitialized == false )
				Printf( "Missing: z\n" );
		}

	protected:
		AActor *missile;
		int lineId;
		fixed_t x;
		fixed_t y;
		fixed_t z;
		bool _missileInitialized;
		bool _lineIdInitialized;
		bool _xInitialized;
		bool _yInitialized;
		bool _zInitialized;
	};

	class WeaponSound : public BaseServerCommand
	{
	public:
		WeaponSound() :
			_playerInitialized( false ),
			_soundInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetSound( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _soundInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _soundInitialized == false )
				Printf( "Missing: sound\n" );
		}

	protected:
		player_t *player;
		FString sound;
		bool _playerInitialized;
		bool _soundInitialized;
	};

	class WeaponChange : public BaseServerCommand
	{
	public:
		WeaponChange() :
			_playerInitialized( false ),
			_weaponTypeInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetWeaponType( const PClass * value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _weaponTypeInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _weaponTypeInitialized == false )
				Printf( "Missing: weaponType\n" );
		}

	protected:
		player_t *player;
		const PClass *weaponType;
		bool _playerInitialized;
		bool _weaponTypeInitialized;
	};

	class WeaponRailgun : public BaseServerCommand
	{
	public:
		WeaponRailgun() :
			_sourceInitialized( false ),
			_startInitialized( false ),
			_endInitialized( false ),
			_color1Initialized( false ),
			_color2Initialized( false ),
			_maxdiffInitialized( false ),
			_flagsInitialized( false ),
			_extendedInitialized( false ),
			_angleoffsetInitialized( false ),
			_spawnclassInitialized( false ),
			_durationInitialized( false ),
			_sparsityInitialized( false ),
			_driftInitialized( false ) {}
		void SetSource( AActor * value );
		void SetStart( const FVector3 & value );
		void SetEnd( const FVector3 & value );
		void SetColor1( int value );
		void SetColor2( int value );
		void SetMaxdiff( float value );
		void SetFlags( int value );
		void SetExtended( bool value );
		void SetAngleoffset( int value );
		void SetSpawnclass( const PClass * value );
		void SetDuration( int value );
		void SetSparsity( float value );
		void SetDrift( float value );
		bool CheckExtended() const;
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _sourceInitialized
				&& _startInitialized
				&& _endInitialized
				&& _color1Initialized
				&& _color2Initialized
				&& _maxdiffInitialized
				&& _flagsInitialized
				&& _extendedInitialized
				&& _angleoffsetInitialized
				&& _spawnclassInitialized
				&& _durationInitialized
				&& _sparsityInitialized
				&& _driftInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _sourceInitialized == false )
				Printf( "Missing: source\n" );
			if ( _startInitialized == false )
				Printf( "Missing: start\n" );
			if ( _endInitialized == false )
				Printf( "Missing: end\n" );
			if ( _color1Initialized == false )
				Printf( "Missing: color1\n" );
			if ( _color2Initialized == false )
				Printf( "Missing: color2\n" );
			if ( _maxdiffInitialized == false )
				Printf( "Missing: maxdiff\n" );
			if ( _flagsInitialized == false )
				Printf( "Missing: flags\n" );
			if ( _extendedInitialized == false )
				Printf( "Missing: extended\n" );
			if ( _angleoffsetInitialized == false )
				Printf( "Missing: angleoffset\n" );
			if ( _spawnclassInitialized == false )
				Printf( "Missing: spawnclass\n" );
			if ( _durationInitialized == false )
				Printf( "Missing: duration\n" );
			if ( _sparsityInitialized == false )
				Printf( "Missing: sparsity\n" );
			if ( _driftInitialized == false )
				Printf( "Missing: drift\n" );
		}

	protected:
		AActor *source;
		FVector3 start;
		FVector3 end;
		int color1;
		int color2;
		float maxdiff;
		int flags;
		bool extended;
		int angleoffset;
		const PClass *spawnclass;
		int duration;
		float sparsity;
		float drift;
		bool _sourceInitialized;
		bool _startInitialized;
		bool _endInitialized;
		bool _color1Initialized;
		bool _color2Initialized;
		bool _maxdiffInitialized;
		bool _flagsInitialized;
		bool _extendedInitialized;
		bool _angleoffsetInitialized;
		bool _spawnclassInitialized;
		bool _durationInitialized;
		bool _sparsityInitialized;
		bool _driftInitialized;
	};

	class SetWeaponZoomFactor : public BaseServerCommand
	{
	public:
		SetWeaponZoomFactor() :
			_playerInitialized( false ),
			_zoomInitialized( false ),
			_flagsInitialized( false ) {}
		void SetPlayer( player_t * value );
		void SetZoom( float value );
		void SetFlags( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _zoomInitialized
				&& _flagsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _zoomInitialized == false )
				Printf( "Missing: zoom\n" );
			if ( _flagsInitialized == false )
				Printf( "Missing: flags\n" );
		}

	protected:
		player_t *player;
		float zoom;
		int flags;
		bool _playerInitialized;
		bool _zoomInitialized;
		bool _flagsInitialized;
	};

	class ACSScriptExecute : public BaseServerCommand
	{
	public:
		ACSScriptExecute() :
			_netidInitialized( false ),
			_activatorInitialized( false ),
			_lineidInitialized( false ),
			_levelnumInitialized( false ),
			_arg0Initialized( false ),
			_arg1Initialized( false ),
			_arg2Initialized( false ),
			_arg3Initialized( false ),
			_backSideInitialized( false ),
			_alwaysInitialized( false ) {}
		void SetNetid( int value );
		void SetActivator( AActor * value );
		void SetLineid( int value );
		void SetLevelnum( int value );
		void SetArg0( int value );
		void SetArg1( int value );
		void SetArg2( int value );
		void SetArg3( int value );
		void SetBackSide( bool value );
		void SetAlways( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _netidInitialized
				&& _activatorInitialized
				&& _lineidInitialized
				&& _levelnumInitialized
				&& _arg0Initialized
				&& _arg1Initialized
				&& _arg2Initialized
				&& _arg3Initialized
				&& _backSideInitialized
				&& _alwaysInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _netidInitialized == false )
				Printf( "Missing: netid\n" );
			if ( _activatorInitialized == false )
				Printf( "Missing: activator\n" );
			if ( _lineidInitialized == false )
				Printf( "Missing: lineid\n" );
			if ( _levelnumInitialized == false )
				Printf( "Missing: levelnum\n" );
			if ( _arg0Initialized == false )
				Printf( "Missing: arg0\n" );
			if ( _arg1Initialized == false )
				Printf( "Missing: arg1\n" );
			if ( _arg2Initialized == false )
				Printf( "Missing: arg2\n" );
			if ( _arg3Initialized == false )
				Printf( "Missing: arg3\n" );
			if ( _backSideInitialized == false )
				Printf( "Missing: backSide\n" );
			if ( _alwaysInitialized == false )
				Printf( "Missing: always\n" );
		}

	protected:
		int netid;
		AActor *activator;
		int lineid;
		int levelnum;
		int arg0;
		int arg1;
		int arg2;
		int arg3;
		bool backSide;
		bool always;
		bool _netidInitialized;
		bool _activatorInitialized;
		bool _lineidInitialized;
		bool _levelnumInitialized;
		bool _arg0Initialized;
		bool _arg1Initialized;
		bool _arg2Initialized;
		bool _arg3Initialized;
		bool _backSideInitialized;
		bool _alwaysInitialized;
	};

	class ACSSendString : public BaseServerCommand
	{
	public:
		ACSSendString() :
			_netidInitialized( false ),
			_activatorInitialized( false ),
			_stringInitialized( false ) {}
		void SetNetid( int value );
		void SetActivator( AActor * value );
		void SetString( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _netidInitialized
				&& _activatorInitialized
				&& _stringInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _netidInitialized == false )
				Printf( "Missing: netid\n" );
			if ( _activatorInitialized == false )
				Printf( "Missing: activator\n" );
			if ( _stringInitialized == false )
				Printf( "Missing: string\n" );
		}

	protected:
		int netid;
		AActor *activator;
		FString string;
		bool _netidInitialized;
		bool _activatorInitialized;
		bool _stringInitialized;
	};

	class SyncJoinQueue : public BaseServerCommand
	{
	public:
		SyncJoinQueue() :
			_slotsInitialized( false ) {}
		void SetSlots( const TArray<struct JoinSlot> & value );
		void PushToSlots(const struct JoinSlot & value)
		{
			slots.Push(value);
			_slotsInitialized = true;
		}
		bool PopFromSlots(struct JoinSlot& value)
		{
			return slots.Pop(value);
		}
		void ClearSlots()
		{
			slots.Clear();
		}
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _slotsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _slotsInitialized == false )
				Printf( "Missing: slots\n" );
		}

	protected:
		TArray<struct JoinSlot> slots;
		bool _slotsInitialized;
	};

	class SyncMapRotation : public BaseServerCommand
	{
	public:
		SyncMapRotation() :
			_entriesInitialized( false ),
			_currentPositionInitialized( false ) {}
		void SetEntries( const TArray<struct MapRotationEntry> & value );
		void PushToEntries(const struct MapRotationEntry & value)
		{
			entries.Push(value);
			_entriesInitialized = true;
		}
		bool PopFromEntries(struct MapRotationEntry& value)
		{
			return entries.Pop(value);
		}
		void ClearEntries()
		{
			entries.Clear();
		}
		void SetCurrentPosition( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _entriesInitialized
				&& _currentPositionInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _entriesInitialized == false )
				Printf( "Missing: entries\n" );
			if ( _currentPositionInitialized == false )
				Printf( "Missing: currentPosition\n" );
		}

	protected:
		TArray<struct MapRotationEntry> entries;
		int currentPosition;
		bool _entriesInitialized;
		bool _currentPositionInitialized;
	};

	class SyncPlayerMedalCounts : public BaseServerCommand
	{
	public:
		SyncPlayerMedalCounts() :
			_playerInitialized( false ),
			_medalsInitialized( false ) {}
		void SetPlayer( int value );
		void SetMedals( const TArray<struct Medal> & value );
		void PushToMedals(const struct Medal & value)
		{
			medals.Push(value);
			_medalsInitialized = true;
		}
		bool PopFromMedals(struct Medal& value)
		{
			return medals.Pop(value);
		}
		void ClearMedals()
		{
			medals.Clear();
		}
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _playerInitialized
				&& _medalsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _medalsInitialized == false )
				Printf( "Missing: medals\n" );
		}

	protected:
		int player;
		TArray<struct Medal> medals;
		bool _playerInitialized;
		bool _medalsInitialized;
	};

	class ReplaceTextures : public BaseServerCommand
	{
	public:
		ReplaceTextures() :
			_fromTextureInitialized( false ),
			_toTextureInitialized( false ),
			_textureFlagsInitialized( false ) {}
		void SetFromTexture( const FString & value );
		void SetToTexture( const FString & value );
		void SetTextureFlags( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _fromTextureInitialized
				&& _toTextureInitialized
				&& _textureFlagsInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _fromTextureInitialized == false )
				Printf( "Missing: fromTexture\n" );
			if ( _toTextureInitialized == false )
				Printf( "Missing: toTexture\n" );
			if ( _textureFlagsInitialized == false )
				Printf( "Missing: textureFlags\n" );
		}

	protected:
		FString fromTexture;
		FString toTexture;
		int textureFlags;
		bool _fromTextureInitialized;
		bool _toTextureInitialized;
		bool _textureFlagsInitialized;
	};

	class SetCustomPlayerValue : public BaseServerCommand
	{
	public:
		SetCustomPlayerValue() :
			_indexInitialized( false ),
			_playerInitialized( false ),
			_valueInitialized( false ) {}
		void SetIndex( int value );
		void SetPlayer( int value );
		void SetValue( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _indexInitialized
				&& _playerInitialized
				&& _valueInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _indexInitialized == false )
				Printf( "Missing: index\n" );
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
			if ( _valueInitialized == false )
				Printf( "Missing: value\n" );
		}

	protected:
		int index;
		int player;
		FString value;
		bool _indexInitialized;
		bool _playerInitialized;
		bool _valueInitialized;
	};

	class ResetCustomPlayerValue : public BaseServerCommand
	{
	public:
		ResetCustomPlayerValue() :
			_indexInitialized( false ),
			_playerInitialized( false ) {}
		void SetIndex( int value );
		void SetPlayer( int value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _indexInitialized
				&& _playerInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _indexInitialized == false )
				Printf( "Missing: index\n" );
			if ( _playerInitialized == false )
				Printf( "Missing: player\n" );
		}

	protected:
		int index;
		int player;
		bool _indexInitialized;
		bool _playerInitialized;
	};

	class OpenMenu : public BaseServerCommand
	{
	public:
		OpenMenu() :
			_menuInitialized( false ) {}
		void SetMenu( const FString & value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _menuInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _menuInitialized == false )
				Printf( "Missing: menu\n" );
		}

	protected:
		FString menu;
		bool _menuInitialized;
	};

	class CloseMenu : public BaseServerCommand
	{
	public:
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return true;
		}
		void PrintMissingParameters() const
		{
		}

	protected:
	};

	class SetDominationPointOwner : public BaseServerCommand
	{
	public:
		SetDominationPointOwner() :
			_pointInitialized( false ),
			_teamInitialized( false ),
			_broadcastInitialized( false ) {}
		void SetPoint( int value );
		void SetTeam( int value );
		void SetBroadcast( bool value );
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseServerCommand( SVC, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _pointInitialized
				&& _teamInitialized
				&& _broadcastInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _pointInitialized == false )
				Printf( "Missing: point\n" );
			if ( _teamInitialized == false )
				Printf( "Missing: team\n" );
			if ( _broadcastInitialized == false )
				Printf( "Missing: broadcast\n" );
		}

	protected:
		int point;
		int team;
		bool broadcast;
		bool _pointInitialized;
		bool _teamInitialized;
		bool _broadcastInitialized;
	};

	class SetDominationPointState : public BaseServerCommand
	{
	public:
		SetDominationPointState() :
			_pointInitialized( false ),
			_disabledInitialized( false ),
			_contestersInitialized( false ) {}
		void SetPoint( int value );
		void SetDisabled( bool value );
		void SetContesters( const TArray<int> & value );
		void PushToContesters(int value)
		{
			contesters.Push(value);
			_contestersInitialized = true;
		}
		bool PopFromContesters(int& value)
		{
			return contesters.Pop(value);
		}
		void ClearContesters()
		{
			contesters.Clear();
		}
		void Execute();
		NetCommand BuildNetCommand() const;
		friend bool ::CLIENT_ParseExtendedServerCommand( SVC2, BYTESTREAM_s * );
		bool AllParametersInitialized() const
		{
			return _pointInitialized
				&& _disabledInitialized
				&& _contestersInitialized;
		}
		void PrintMissingParameters() const
		{
			if ( _pointInitialized == false )
				Printf( "Missing: point\n" );
			if ( _disabledInitialized == false )
				Printf( "Missing: disabled\n" );
			if ( _contestersInitialized == false )
				Printf( "Missing: contesters\n" );
		}

	protected:
		int point;
		bool disabled;
		TArray<int> contesters;
		bool _pointInitialized;
		bool _disabledInitialized;
		bool _contestersInitialized;
	};

}
