#ifdef SOUND_DSOUND

#define DIRECTSOUND_VERSION 0x0700
#include <dsound.h>

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>

#include "main.h"
#include "util.h"
#include "file.h"
#include "sound.h"

LPDIRECTSOUND			lpDS = NULL;
LPDIRECTSOUND3DLISTENER	lpDS3DListener = NULL;
LPDIRECTSOUNDBUFFER		glpPrimaryBuffer = NULL;

#define _HSNDOBJ_DEFINED

// globals
BOOL Sound3D = FALSE;
BOOL FreeHWBuffers = TRUE;

//
// Generic Functions
//

// Initializes main globals
// a. A DirectSound Object
// b. A DirectSound3DListener Object
// c. A Primary Buffer.
BOOL sound_init( void )
{
	DSBUFFERDESC dsbdesc;
	int iErr;
	lpDS = NULL;
	if FAILED( CoInitialize(NULL) )
		return FALSE;

	// Attempt to initialize with DirectSound.
	// First look for DSOUND.DLL using CoCreateInstance.
	iErr = CoCreateInstance(&CLSID_DirectSound, NULL, CLSCTX_INPROC_SERVER,
								 &IID_IDirectSound, (void **) &lpDS);
	
		
	if ((iErr >= DS_OK)	&& (lpDS)) // Found DSOUND.DLL
		iErr = IDirectSound_Initialize(lpDS, NULL);	// Try to init Direct Sound.

	if (iErr < DS_OK)
		return FALSE; // Failed to get DirectSound, so no sound-system available.

	// build sound_caps structure
	{
		DSCAPS DSCaps;
		memset (&DSCaps, 0, sizeof (DSCAPS));
		DSCaps.dwSize = sizeof(DSCAPS);
		IDirectSound_GetCaps( lpDS, &DSCaps );
		sound_caps.memory = DSCaps.dwMaxContigFreeHwMemBytes;
		sound_caps.buffers = DSCaps.dwFreeHwMixingStaticBuffers;
		sound_caps.min_volume = ( DSBVOLUME_MIN / 3);
	}

	// Succeeded in getting DirectSound.
	// Check to see if there is 3D acceleration.
	/*
	DSCAPS	dsCaps;
	dsCaps.dwSize = sizeof(DSCAPS);
	IDirectSound_GetCaps(lpDS, &dsCaps);
	// Allow 3D sound only if acceleration exists.
	Sound3D = ((dsCaps.dwMaxHw3DAllBuffers > 0) ? TRUE : FALSE);
	*/

	// If here, got a valid sound-interface...
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
	dsbdesc.dwBufferBytes = 0; //dwBufferBytes and lpwfxFormat must be set this way.
	dsbdesc.lpwfxFormat = NULL;
	
	// Set control-level of DirectSound. (To normal, default.)
	if (IDirectSound_SetCooperativeLevel(lpDS, GetActiveWindow(), DSSCL_EXCLUSIVE /*DSSCL_NORMAL*/) >= DS_OK)    
	{
		// Create primary buffer.
		if ( IDirectSound_CreateSoundBuffer( lpDS, &dsbdesc, &glpPrimaryBuffer, NULL ) == DS_OK )
		{
			
			DWORD dwSizeWritten;
			LPWAVEFORMATEX lpwaveinfo;
			
			IDirectSoundBuffer_GetFormat( glpPrimaryBuffer, NULL, 0, &dwSizeWritten );
			lpwaveinfo = (LPWAVEFORMATEX)malloc( dwSizeWritten );
			IDirectSoundBuffer_GetFormat( glpPrimaryBuffer, lpwaveinfo, dwSizeWritten, 0 );
			
			lpwaveinfo->nChannels = 2;
			lpwaveinfo->nSamplesPerSec = 22050;
			lpwaveinfo->wBitsPerSample = 16;
			lpwaveinfo->nBlockAlign = 4;
			lpwaveinfo->nAvgBytesPerSec = lpwaveinfo->nSamplesPerSec * lpwaveinfo->nBlockAlign;

			if ( IDirectSoundBuffer_SetFormat( glpPrimaryBuffer, lpwaveinfo ) != DS_OK )
			{
				free(lpwaveinfo);
				return FALSE;
			}

			IDirectSoundBuffer_GetFormat( glpPrimaryBuffer, lpwaveinfo, dwSizeWritten, 0 );
			DebugPrintf("using primary buffer format: wFormatTag %d, nChannels %d, nSamplesPerSec %d, nAvgBytesPerSec %d, nBlockAlign %d, wBitsPerSample %d\n",
				lpwaveinfo->wFormatTag, lpwaveinfo->nChannels, lpwaveinfo->nSamplesPerSec, lpwaveinfo->nAvgBytesPerSec, lpwaveinfo->nBlockAlign, lpwaveinfo->wBitsPerSample );
			
			free(lpwaveinfo);

			// If no 3D, we are done.
			if (!Sound3D)
				return(TRUE);

			// If 3D, need to get listener interface.
			if (IDirectSoundBuffer_QueryInterface(glpPrimaryBuffer, &IID_IDirectSound3DListener, (void **) &lpDS3DListener) >= DS_OK)
			{
				// Start primary-buffer looped-playing to reduce overhead on secondary-play calls.
				glpPrimaryBuffer->lpVtbl->Play(glpPrimaryBuffer, 0, 0, DSBPLAY_LOOPING);
				return(TRUE);
			}
			else
			{
				// Failed to get 3D, so we only have 2D-controls. Release listener if valid, and go with 2D.
				if (lpDS3DListener)
				{
					IDirectSound3DListener_Release(lpDS3DListener);
					lpDS3DListener = NULL;
				}
				
				// Flag all 3D as off.
				Sound3D = FALSE;

				return(TRUE);	// Still succeed initialization, but with 2D.
			}
		}
	}

	// If here, failed to initialize sound system in some way. (Either in SetCoopLevel, or creating primary-buffer.)
	IDirectSound_Release(lpDS);
	lpDS = NULL;
	
	DebugPrintf("returning FALSE from Init_SoundGlobals at point 2\n");

	return(FALSE);
}

void sound_commit_any_pending( void )
{
	if ( !lpDS3DListener )
		return;
	IDirectSound3DListener_CommitDeferredSettings(
		lpDS3DListener
	);
}

//
// Listener
//

BOOL sound_listener_position( float x, float y, float z )
{
	return IDirectSound3DListener_SetPosition(
		lpDS3DListener,	
		x, y, z, 
		DS3D_DEFERRED
	) == DS_OK;
}

BOOL sound_listener_velocity( float x, float y, float z )
{
	return IDirectSound3DListener_SetVelocity(
		lpDS3DListener,	
		x, y, z, 
		DS3D_DEFERRED
	) == DS_OK;
}

BOOL sound_listener_orientation( 
	float fx, float fy, float fz, // forward vector
	float ux, float uy, float uz  // up vector
)
{
	return IDirectSound3DListener_SetOrientation(
		lpDS3DListener,	
		fx, fy, fz, 
		ux, uy, uz, 
		DS3D_DEFERRED
	) == DS_OK;
}

//
// Buffers
//

void sound_destroy( void )
{
	if ( !lpDS )
		return;
	sound_buffer_release( glpPrimaryBuffer );
	IDirectSound_Release(lpDS);
}

void sound_buffer_play( void * buffer )
{
	IDirectSoundBuffer_Play(
		(IDirectSoundBuffer*)buffer, 0, 0, 0 
	);
}

void sound_buffer_play_looping( void * buffer )
{
	IDirectSoundBuffer_Play(
		(IDirectSoundBuffer*)buffer, 0, 0,
		DSBPLAY_LOOPING 
	);
}

void sound_buffer_stop( void * buffer )
{
	IDirectSoundBuffer_Stop( 
		(IDirectSoundBuffer*) buffer 
	);
}

DWORD sound_buffer_size( void * buffer )
{
	DSBCAPS dsbcaps; 
	dsbcaps.dwSize = sizeof( DSBCAPS );
	IDirectSoundBuffer_GetCaps(
		(IDirectSoundBuffer*)buffer,
		&dsbcaps 
	);
	return dsbcaps.dwBufferBytes;
}

void sound_buffer_release( void * ptr )
{
	IDirectSoundBuffer* buffer = ptr;
	if (buffer != NULL)
		return;
	buffer->lpVtbl->Release(buffer);
	buffer = NULL;
}

void sound_buffer_3d_release( void * buffer )
{
	IDirectSound3DBuffer_Release(
		(IDirectSound3DBuffer*) buffer 
	);
}

BOOL sound_buffer_is_playing( void * buffer )
{
	DWORD dwStatus;
	IDirectSoundBuffer_GetStatus(
		(IDirectSoundBuffer*)buffer, 
		&dwStatus 
	);
	return (dwStatus & DSBSTATUS_PLAYING);
}

void sound_buffer_set_freq( void* ptr, float freq )
{
	IDirectSoundBuffer * buffer = ptr;
	LPWAVEFORMATEX lpwaveinfo;
	DWORD dwSizeWritten, OrigFreq;

	// BUG:  Appears buffer pointer goes bad or is passed in as NULL
	if(!buffer)
	{
		DebugPrintf("BUG: sound_buffer_set_freq() buffer passed in was null\n");
		return;
	}

	if ( !freq || ( freq == 1.0F ) )
	{
		OrigFreq = DSBFREQUENCY_ORIGINAL; 
	}
	else
	{
		// get original frequency of buffer
		IDirectSoundBuffer_GetFormat( buffer, NULL, 0, &dwSizeWritten );
		lpwaveinfo = (LPWAVEFORMATEX)malloc( dwSizeWritten );
		IDirectSoundBuffer_GetFormat( buffer, lpwaveinfo, dwSizeWritten, 0 );
		OrigFreq = lpwaveinfo->nSamplesPerSec; 
		free(lpwaveinfo);
	
		// work out new frequency
		OrigFreq = (DWORD)( (float)OrigFreq * freq );

		if ( OrigFreq < DSBFREQUENCY_MIN )
			OrigFreq = DSBFREQUENCY_MIN;

		if ( OrigFreq > DSBFREQUENCY_MAX )
			OrigFreq = DSBFREQUENCY_MAX;
	}

	// set frequency
	if ( IDirectSoundBuffer_SetFrequency( buffer, OrigFreq ) != DS_OK )
		DebugPrintf("sound_buffer_set_freq: failed\n");
}

void sound_buffer_volume( void * buffer, long volume )
{
	IDirectSoundBuffer_SetVolume( (IDirectSoundBuffer*) buffer, volume );
}

void sound_buffer_pan( void * buffer, long pan )
{
	IDirectSoundBuffer_SetPan( (IDirectSoundBuffer*) buffer, pan );
}

DWORD sound_buffer_get_freq( void * buffer ) // samples per sec
{
	LPWAVEFORMATEX lpwaveinfo;
	DWORD dwSizeWritten, freq;
	IDirectSoundBuffer_GetFormat(
		(IDirectSoundBuffer*) buffer,
		NULL, 0, &dwSizeWritten 
	);
	lpwaveinfo = (LPWAVEFORMATEX)malloc( dwSizeWritten );
	IDirectSoundBuffer_GetFormat( 
		(IDirectSoundBuffer*) buffer,
		lpwaveinfo, dwSizeWritten, 0 
	);
	freq = lpwaveinfo->nSamplesPerSec; 
	free(lpwaveinfo);
	return freq;
}

DWORD sound_buffer_get_rate( void * buffer ) // avg bytes per second
{
	LPWAVEFORMATEX lpwaveinfo;
	DWORD dwSizeWritten, datarate;
	IDirectSoundBuffer_GetFormat( 
		(IDirectSoundBuffer*) buffer, 
		NULL, 0, &dwSizeWritten 
	);
	lpwaveinfo = (LPWAVEFORMATEX)malloc( dwSizeWritten );
	IDirectSoundBuffer_GetFormat( 
		(IDirectSoundBuffer*) buffer, 
		lpwaveinfo, dwSizeWritten, 0 
	);
	datarate = lpwaveinfo->nAvgBytesPerSec; 
	free(lpwaveinfo);
	return datarate;
}

// this gets the current play location
void sound_buffer_get_position( void * buffer, DWORD* time )
{
	IDirectSoundBuffer_GetCurrentPosition(
		(IDirectSoundBuffer*) buffer,
		time,
		NULL
	);
}

// this moves to a specific offset in the buffer
void sound_buffer_set_position( void * buffer, DWORD time )
{
	IDirectSoundBuffer_SetCurrentPosition(
		(IDirectSoundBuffer*) buffer,
		time
	);
}

// this sets the location in 3d space of the sound
void sound_buffer_set_3d_position( void * buffer, float x, float y, float z, float min, float max )
{			
	IDirectSound3DBuffer_SetPosition(
		(IDirectSound3DBuffer*) buffer,
		x, y, z, DS3D_IMMEDIATE
	);
	IDirectSound3DBuffer_SetMinDistance(
		(IDirectSound3DBuffer*) buffer,
		min, DS3D_IMMEDIATE
	); 
	IDirectSound3DBuffer_SetMaxDistance(
		(IDirectSound3DBuffer*) buffer,
		max, DS3D_IMMEDIATE
	); 
}

void* sound_buffer_load(char *name)
{
    IDirectSoundBuffer *sound_buffer = NULL;
    DSBUFFERDESC buffer_description = {0};
	WAVEFORMATEX buffer_format;
	SDL_AudioSpec wav_spec;
	Uint32 wav_length;
	Uint8 *wav_buffer;
	DWORD flags = DSBCAPS_STATIC | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_LOCSOFTWARE;

	if (!lpDS)
		return NULL;

	if( SDL_LoadWAV(name, &wav_spec, &wav_buffer, &wav_length) == NULL )
	{
		DebugPrintf("Could not open test.wav: %s\n", SDL_GetError());
		return NULL;
	}

	// http://msdn.microsoft.com/en-us/library/ms897764.aspx
    buffer_description.dwSize			= sizeof(buffer_description);
    buffer_description.dwFlags			= flags;
	buffer_description.dwBufferBytes	= (DWORD) wav_length;
	buffer_description.lpwfxFormat		= &buffer_format;

	// http://msdn.microsoft.com/en-us/library/dd757720%28VS.85%29.aspx
	buffer_format.wFormatTag		= (WORD)	WAVE_FORMAT_PCM;
	buffer_format.nChannels			= (WORD)	wav_spec.channels;
	buffer_format.wBitsPerSample	= (WORD)	((wav_spec.format == AUDIO_U8 || wav_spec.format == AUDIO_S8) ? 8 : 16);
	buffer_format.nSamplesPerSec	= (DWORD)	wav_spec.freq;
	buffer_format.nBlockAlign		= (WORD)	(buffer_format.nChannels * buffer_format.wBitsPerSample) / 8;
	buffer_format.nAvgBytesPerSec	= (DWORD)	(buffer_format.nSamplesPerSec * buffer_format.nBlockAlign);
	buffer_format.cbSize			= (WORD)	0;

	// http://msdn.microsoft.com/en-us/library/ms898123.aspx
	if( IDirectSound_CreateSoundBuffer( lpDS, &buffer_description, &sound_buffer, NULL ) == DS_OK )
    {
        LPVOID pMem1, pMem2;
        DWORD dwSize1, dwSize2;
        if (SUCCEEDED(IDirectSoundBuffer_Lock(sound_buffer, 0, wav_length, &pMem1, &dwSize1, &pMem2, &dwSize2, 0)))
        {
            CopyMemory(pMem1, wav_buffer, dwSize1);
            if ( 0 != dwSize2 )
                CopyMemory(pMem2, wav_buffer+dwSize1, dwSize2);
            IDirectSoundBuffer_Unlock(sound_buffer, pMem1, dwSize1, pMem2, dwSize2);
        }
		else
		{
            sound_buffer_release(sound_buffer);
            sound_buffer = NULL;
		}
    }
    else
    {
        sound_buffer = NULL;
    }

	SDL_FreeWAV(wav_buffer);

    return sound_buffer;
}

//
// Sources
//

void sound_source_destroy( sound_source_t * source )
{
	int i;
	if (!source)
		return;
	for (i = 0; i < MAX_DUP_BUFFERS; i++)
	{
		if (source->Dup_Buffer[i])
		{
            sound_buffer_release(source->Dup_Buffer[i]);
	        source->Dup_Buffer[i] = NULL;
		}
	}
    free(source);
}

sound_source_t *sound_source_create(char *path, int sfx_flags, int sfx)
{
    sound_source_t *pSO = NULL;
	void * Buffer = NULL;
	int i;
	DSBCAPS dsbcaps;

    pSO = (sound_source_t *)malloc(sizeof(sound_source_t));

    if (!pSO)
		return NULL;

	memset( pSO, 0, sizeof(sound_source_t) );

	pSO->looping_sfx_index[0] = -1;
	pSO->Dup_Buffer[0] = sound_buffer_load(path);

	if( !pSO->Dup_Buffer[ 0 ] )
	{
		Msg("Unable to create sound buffer for %s\n", path );
		sound_source_destroy( pSO );
		return pSO;
	}

	// get caps of buffer
	memset( &dsbcaps, 0, sizeof( DSBCAPS ) );
	dsbcaps.dwSize = sizeof( DSBCAPS );
	IDirectSoundBuffer_GetCaps( (IDirectSoundBuffer*)pSO->Dup_Buffer[ 0 ], &dsbcaps );

	// if buffer in hw, should check free channels here, but Ensoniq driver always reports back 256
	// so we will just have to try duplicating until failure
	for (i = 1; i < MAX_DUP_BUFFERS; i++)
	{
		pSO->looping_sfx_index[i] = -1;
		// duplicate 2D buffer...
		if ( !IDirectSound_DuplicateSoundBuffer( 
			lpDS, 
			(LPDIRECTSOUNDBUFFER)pSO->Dup_Buffer[0], 
			(LPDIRECTSOUNDBUFFER*)&pSO->Dup_Buffer[i] 
		) == DS_OK )
		{
			DebugPrintf("unable to duplicate sound buffer\n");

			// invalidate buffer & all duplicates
			sound_source_destroy( pSO );

			// was original buffer hw? if so, try to recreate in sw
			if ( dsbcaps.dwFlags & DSBCAPS_LOCHARDWARE )
			{
				DebugPrintf("trying to recreate in sw\n");
				FreeHWBuffers = FALSE;

				// recreate all buffers up to & including this one in software
				pSO->Dup_Buffer[ 0 ] = sound_buffer_load(path);

				if( !pSO->Dup_Buffer[ 0 ] )
				{
					Msg("Unable to create sound buffer for %s\n", path );
					sound_source_destroy( pSO );
					return pSO;
				}

				i = 0;
				continue;
			}
			else
			{
				// couldn't duplicate buffer in sw - just break out with buffer info still marked as invalid
				Msg("unable to duplicate buffers in sw\n");
				break;
			}
		}
	}
    return pSO;
}

#endif // SOUND_DSOUND
