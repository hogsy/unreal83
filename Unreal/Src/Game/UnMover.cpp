/*=============================================================================
	UnMover.cpp: Keyframe mover actor code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "UnGame.h"
#include "UnDynBsp.h"
#include "UnFActor.h"

/*-----------------------------------------------------------------------------
	Mover base class
-----------------------------------------------------------------------------*/

int AMover::Process(ILevel *Level, FName Message, void *Params)
	{
	GUARD;
	AActor *This = (AActor *)this;
    FActor & Actor = FActor::Actor(*this);
	//
	switch (Message.Index)
		{
		case ACTOR_BeginPlay:
			{
			if (KeyNum>3) KeyNum=3;
			//
			bMoving    = 0;
			CurTime    = 0;
			HoldTime   = 0;
			PrevKeyNum = KeyNum;
			//
			Location = BasePos + KeyPos[KeyNum];
			DrawRot  = BaseRot + KeyRot[KeyNum];
			//
			// Turn off actor collision checking if this mover doesn't need it:
			//
			if ((MoverTriggerType!=MT_ProximityOpenTimed) &&
				(MoverTriggerType!=MT_ProximityControl) &&
				(!bTrigger))
				{
				Actor.SetActorCollision(FALSE);
				};
			};
			return 1;
		case ACTOR_KeyMoveTo:
			{
			PKeyMove *KeyInfo = (PKeyMove *)Params;
			if (!bMoving) // Start moving
				{
				PrevKeyNum		 = KeyNum;
				KeyNum			 = KeyInfo->KeyNum;
				bMoving			 = 1;
				CurTime			 = 0;
				HoldTime		 = 0;
				bReverseWhenDone = 0;
                Actor.MakeSound(OpenSound);
                Actor.MakeSound(MoveAmbientSound);
				}
			else if (bCanInterruptMove) // Reverse course smoothly
				{
				Level->SendMessage(iMe,ACTOR_KeyReverse,NULL);
				bReverseWhenDone = 0;
				}
			else bReverseWhenDone ^= 1;
			return 1;
			};
		case ACTOR_KeyStop:
			if (bMoving)
				{
				bMoving	= 0;
				};
			return 1;
		case ACTOR_KeyReverse:
			if (bMoving)
				{
				BYTE Temp	= KeyNum;
				KeyNum		= PrevKeyNum;
				PrevKeyNum	= Temp;
				CurTime		= MoverTime - CurTime;
				};
			return 1;
		case ACTOR_SetKeyPoint:
			// Must implement !!
			return 1;
		case ACTOR_Tick:
			//
			DrawRot += FreeRotation;
			if (bMoving) // Moving
				{
				if (++CurTime <= MoverTime) // Continue moving
					{
					FLOAT Alpha = (FLOAT)CurTime / (FLOAT)MoverTime;
					if (MoverGlideType==MV_GlideByTime)
						{
						// Make alpha time-smooth and time-continuous
						Alpha = 3.0*Alpha*Alpha - 2.0*Alpha*Alpha*Alpha;
						};
					Location = BasePos + KeyPos[PrevKeyNum] + (KeyPos[KeyNum]-KeyPos[PrevKeyNum]) * Alpha;
					DrawRot  = BaseRot + KeyRot[PrevKeyNum] + (KeyRot[KeyNum]-KeyRot[PrevKeyNum]) * Alpha;
					}
				else // Just finished moving
					{
					bMoving  = 0;
					CurTime  = 0;
					//
					Location = BasePos + KeyPos[KeyNum];
					DrawRot  = BaseRot + KeyRot[KeyNum];
					//
					if (bReverseWhenDone)
						{
						PKeyMove KeyInfo;
						KeyInfo.KeyNum = KeyNum ? 0 : 1;
						Level->SendMessage(iMe,ACTOR_KeyMoveTo,&KeyInfo);
						}
                    else
                        {
                        GAudio.SfxStopActor(iMe); // Stop any current sound
                        Actor.MakeSound(ClosedSound);
                        }
					}
				}
			else // Not moving
				{
				switch (MoverTriggerType)
					{
					case MT_TriggerOpenTimed:
					case MT_ProximityOpenTimed:
					case MT_StandOpenTimed:
						// If open and timer is active, wait till timer expires then close
						if ((KeyNum==1) && (HoldTime!=0))
							{
							if (++HoldTime >= RemainOpenTime) // Time expired, now close
								{
								PKeyMove KeyInfo;
								KeyInfo.KeyNum = 0;
								Level->SendMessage(iMe,ACTOR_KeyMoveTo,&KeyInfo);
								};
							};
						break;
					};
				};
			if (Brush && (Brush->Location != Location) || (Brush->Rotation != DrawRot))
				{
				for (int i=0; i<16; i++)
					{
					if (iSlaves[i]!=INDEX_NONE)
						{
						AActor *Slave = &Level->Actors->Element(iSlaves[i]);
						Slave->Location    += Location    - Brush->Location;
						Slave->DrawRot     += DrawRot     - Brush->Rotation;
						Slave->ViewRot.Yaw += DrawRot.Yaw - Brush->Rotation.Yaw;
						};
					};
				sporeUpdate(iMe);
				};
			return 1;
		case ACTOR_Trigger:
			{
			PKeyMove KeyInfo;
			switch (MoverTriggerType)
				{
				case MT_TriggerOpenTimed:
				case MT_TriggerControl:
					// Open it
					KeyInfo.KeyNum = 1;
					Level->SendMessage(iMe,ACTOR_KeyMoveTo,&KeyInfo);
					break;
				case MT_TriggerToggle:
					// Toggle it
					KeyInfo.KeyNum = KeyNum ? 0 : 1;
					Level->SendMessage(iMe,ACTOR_KeyMoveTo,&KeyInfo);
					break;
				};
			};
			return 1;
		case ACTOR_UnTrigger:
			{
			PKeyMove KeyInfo;
			switch (MoverTriggerType)
				{
				case MT_TriggerOpenTimed:
					// Start timer ticking down
					if (!HoldTime) HoldTime=1;
					break;
				case MT_TriggerToggle:
					// Ignore untrigger
					break;
				case MT_TriggerControl:
					// Close it
					KeyInfo.KeyNum = 0;
					Level->SendMessage(iMe,ACTOR_KeyMoveTo,&KeyInfo);
					break;
				};
			};
			return 1;
		case ACTOR_Touch:
			{
			if (BumpPlayerDamage != 0.0)
				{
			    //FActor & Mover  = FActor::Actor(*this);
				//FActor & Target = FActor::Actor(((PHit *)Params)->iActor);
				//Mover.CauseDamage(Target,BumpDamage,0.0);
				};
			PKeyMove KeyInfo;
			if (bTrigger && (!bMoving) && (KeyNum==0) && (EventName!=NAME_NONE)) // Trigger other events
				{
				Level->SendMessageEx(ACTOR_Trigger,Params,INDEX_NONE,EventName,NULL);
				if (bTriggerOnceOnly)
					{
					MoverTriggerType = MT_None;
					bTrigger = 0;
					};
				};
			switch (MoverTriggerType)
				{
				case MT_ProximityOpenTimed:
				case MT_ProximityControl:
					// Open it
					KeyInfo.KeyNum = 1;
					Level->SendMessage(iMe,ACTOR_KeyMoveTo,&KeyInfo);
					break;
				};
			};
			return 1;
		case ACTOR_UnTouch:
			PKeyMove KeyInfo;
			if (bTrigger && (EventName!=NAME_NONE)) // Trigger other events
				{
				PTouch TouchParams;
				TouchParams.iActor = iMe;
				Level->SendMessageEx(ACTOR_UnTrigger,&TouchParams,INDEX_NONE,EventName,NULL);
				};
			switch (MoverTriggerType)
				{
				case MT_ProximityOpenTimed:
					// Start timer ticking down
					if (!HoldTime) HoldTime=1;
					break;
				case MT_ProximityControl:
					// Close it
					KeyInfo.KeyNum = 0;
					Level->SendMessage(iMe,ACTOR_KeyMoveTo,&KeyInfo);
					break;
				};
			return 1;
		case ACTOR_Spawn:
			BasePos		= Location;
			BaseRot		= DrawRot;
			PrevKeyNum	= KeyNum;
			return 1;
		case ACTOR_PostEditChange:
			{
			// Validate KeyNum:
			if (KeyNum>=4) KeyNum=3;
			// Update BasePos:
			if (This->IsMovingBrush())
				{
				BasePos  = Brush->Location - OldPos;
				BaseRot  = Brush->Rotation - OldRot;
				}
			else
				{
				BasePos  = Location - OldPos;
				BaseRot  = DrawRot  - OldRot;
				};
			// Update Old:
			OldPos = KeyPos[KeyNum];
			OldRot = KeyRot[KeyNum];
			// Update Location:
			Location = BasePos + OldPos;
			DrawRot  = BaseRot + OldRot;
			// Update PrevKeyNum:
			PrevKeyNum = KeyNum;
			//
			This->UpdateBrushPosition(Level,iMe,0);
			};
			return 1;
		case ACTOR_PostEditMove:
			{
			if (KeyNum==0) // Changing location
				{
				if (This->IsMovingBrush())
					{
					BasePos  = Brush->Location - OldPos;
					BaseRot  = Brush->Rotation - OldRot;
					}
				else
					{
					BasePos  = Location - OldPos;
					BaseRot  = DrawRot  - OldRot;
					};
				}
			else // Changing displacement of KeyPos[KeyNum] relative to KeyPos[0]
				{
				if (This->IsMovingBrush())
					{
					// Update Key:
					KeyPos[KeyNum] = Brush->Location - (BasePos + KeyPos[0]);
					KeyRot[KeyNum] = Brush->Rotation - (BaseRot + KeyRot[0]);
					}
				else
					{
					// Update Key:
					KeyPos[KeyNum] = Location - (BasePos + KeyPos[0]);
					KeyRot[KeyNum] = DrawRot  - (BaseRot + KeyRot[0]);
					};
				// Update Old:
				OldPos = KeyPos[KeyNum];
				OldRot = KeyRot[KeyNum];
				};
			Brush->Location = BasePos + KeyPos[KeyNum];
			return 1;
			};
		case ACTOR_SteppedOn:
			{
			INDEX iActor = *(INDEX *)Params;
			int i;
			for (i=0; i<16; i++)
				{
				if (iSlaves[i]==iActor) break;
				};
			if (i>=16)
				{
				for (i=0; i<16; i++)
					{
					if (iSlaves[i]==INDEX_NONE)
						{
						iSlaves[i]=iActor;
						break;
						};
					};
				};
			PKeyMove KeyInfo;
			switch (MoverTriggerType)
				{
				case MT_StandOpenTimed:
					// Open it
					KeyInfo.KeyNum = 1;
					Level->SendMessage(iMe,ACTOR_KeyMoveTo,&KeyInfo);
					break;
				};
			return 1;
			};
		case ACTOR_UnSteppedOn:
			{
			INDEX iActor = *(INDEX *)Params;
			for (int i=0; i<16; i++)
				{
				if (iSlaves[i]==iActor) iSlaves[i]=INDEX_NONE;
				};
			switch (MoverTriggerType)
				{
				case MT_StandOpenTimed:
					// Start timer ticking down
					if (!HoldTime) HoldTime=1;
					break;
				};
			return 1;
			};
		case ACTOR_PreRaytrace:  // Called before raytracing session beings
		case ACTOR_PostRaytrace: // Called after raytracing session ends
			Location = BasePos + KeyPos[KeyNum];
			DrawRot  = BaseRot + KeyRot[KeyNum];
			sporeUpdate(iMe);
			return 1;
		case ACTOR_RaytraceWorld: // Place this brush in position to raytrace the world
			if (WorldRaytraceKey!=255)
				{
				Location = BasePos + KeyPos[WorldRaytraceKey];
				DrawRot  = BaseRot + KeyRot[WorldRaytraceKey];
				sporeUpdate(iMe);
				}
			else sporeFlush(iMe);
			return 1;
		case ACTOR_RaytraceBrush: // Place this brush in position to raytrace the brush
			if (BrushRaytraceKey!=255)
				{
				Location = BasePos + KeyPos[BrushRaytraceKey];
				DrawRot  = BaseRot + KeyRot[BrushRaytraceKey];
				sporeUpdate(iMe);
				return 1;
				}
			else return 0;
			return 1;
		};
	return 0;
	UNGUARD("AMover::Process");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
