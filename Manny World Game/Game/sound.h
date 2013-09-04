#ifndef SOUND_H
#define SOUND_H

void* soundload( char *fname, long *bufsize );
void* soundload( char *fname, long *bufsize ){
   FILE* fp = fopen( fname, "rb" );
   fseek( fp, 0L, SEEK_END );
   long long len = ftell( fp );
   rewind( fp );
   void *buf = malloc( len );
   fread( buf, 1, len, fp );
   fclose( fp );
   *bufsize = len;
   return buf;
}
void sound(char * a)
{
   float curr[3] = {0.,0.,-1.};
//   float targ[3] = {0.,0.,-1.};

   /* initialize OpenAL context, asking for 44.1kHz to match HRIR data */
   ALCint contextAttr[] = {ALC_FREQUENCY,44100,0};
   ALCdevice* device = alcOpenDevice( NULL );
   ALCcontext* context = alcCreateContext( device, contextAttr );
   alcMakeContextCurrent( context );

   /* listener at origin, facing down -z (ears at 1.5m height) */
   alListener3f( AL_POSITION, 0,0,0 );
   alListener3f( AL_VELOCITY, 0., 0., 0. );
   float orient[6] = { /*fwd:*/ 0., 0., -1., /*up:*/ 0., 1., 0. };
   alListenerfv( AL_ORIENTATION, orient );

   /* this will be the source of ghostly footsteps... */
   ALuint source;
   alGenSources( 1, &source );
   alSourcef( source, AL_PITCH, 1. );
   alSourcef( source, AL_GAIN, 1. );
   alSource3f( source, AL_POSITION, curr[0],curr[1],curr[2] );
   alSource3f( source, AL_VELOCITY, 0.,0.,0. );
   alSourcei( source, AL_LOOPING, AL_TRUE );

   /* allocate an OpenAL buffer and fill it with monaural sample data */
   ALuint buffer;
   alGenBuffers( 1, &buffer );
   {
      long dataSize;

      const ALvoid* data = soundload( a, &dataSize );
      /* for simplicity, assume raw file is signed-16b at 44.1kHz */
      alBufferData( buffer, AL_FORMAT_MONO16, data, dataSize, 44100 );
      free( (void*)data );
   }
   alSourcei( source, AL_BUFFER, buffer );

   /* state initializations for the upcoming loop */
//   /** BEGIN! **/
   alSourcePlay( source );

   fflush( stderr ); /* in case OpenAL reported an error earlier */
}
void soundb(char * a)
{
   float curr[3] = {0.,0.,-1.};
//   float targ[3] = {0.,0.,-1.};

   /* initialize OpenAL context, asking for 44.1kHz to match HRIR data */
   ALCint contextAttr[] = {ALC_FREQUENCY,44100,0};
   ALCdevice* device = alcOpenDevice( NULL );
   ALCcontext* context = alcCreateContext( device, contextAttr );
   alcMakeContextCurrent( context );

   /* listener at origin, facing down -z (ears at 1.5m height) */
   alListener3f( AL_POSITION, 0,0,0 );
   alListener3f( AL_VELOCITY, 0., 0., 0. );
   float orient[6] = { /*fwd:*/ 0., 0., -1., /*up:*/ 0., 1., 0. };
   alListenerfv( AL_ORIENTATION, orient );

   /* this will be the source of ghostly footsteps... */
   ALuint source;
   alGenSources( 1, &source );
   alSourcef( source, AL_PITCH, 1. );
   alSourcef( source, AL_GAIN, 1. );
   alSource3f( source, AL_POSITION, curr[0],curr[1],curr[2] );
   alSource3f( source, AL_VELOCITY, 0.,0.,0. );
   alSourcei( source, AL_LOOPING, AL_FALSE );

   /* allocate an OpenAL buffer and fill it with monaural sample data */
   ALuint buffer;
   alGenBuffers( 1, &buffer );
   {
      long dataSize;

      const ALvoid* data = soundload( a, &dataSize );
      /* for simplicity, assume raw file is signed-16b at 44.1kHz */
      alBufferData( buffer, AL_FORMAT_MONO16, data, dataSize, 44100 );
      free( (void*)data );
   }
   alSourcei( source, AL_BUFFER, buffer );

   /* state initializations for the upcoming loop */
//   /** BEGIN! **/
   alSourcePlay( source );

   fflush( stderr ); /* in case OpenAL reported an error earlier */
}


#endif
