/*
 * 1394-Based Digital Camera Capture Code for the Control Library
 * Written by Chris Urmson <curmson@ri.cmu.edu>
 * Additions by Damien Douxchamps <ddouxchamps@users.sf.net>
 * Additions by David Moore <dcm@acm.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#include <libraw1394/raw1394.h>
#include <libraw1394/csr.h>

#include "config.h"
#include "internal.h"
#include "control.h"
#include "kernel-video1394.h"
#include "utils.h"
#include "linux/linux.h"
#include "linux/capture.h"

#define MAX_NUM_PORTS 16

/* Variables used for simultaneous capture of video from muliple cameras
   This is only used by RAW1394 capture. VIDEO1394 (aka DMA) capture
   doesn't use this */
int *_dc1394_buffer[DC1394_NUM_ISO_CHANNELS];
int _dc1394_frame_captured[DC1394_NUM_ISO_CHANNELS];
int _dc1394_offset[DC1394_NUM_ISO_CHANNELS];
int _dc1394_quadlets_per_frame[DC1394_NUM_ISO_CHANNELS];
int _dc1394_quadlets_per_packet[DC1394_NUM_ISO_CHANNELS];
int _dc1394_all_captured;

/* variables to handle multiple cameras using a single fd. */
int *_dc1394_dma_fd = NULL;
int *_dc1394_num_using_fd = NULL;

/**********************/
/* Internal functions */
/**********************/

/**************************************************************
 _dc1394_video_iso_handler

 This is the routine that is plugged into the raw1394 callback
 hook to allow us to capture mutliple iso video streams.  This
 is used in the non DMA capture routines.
***************************************************************/
dc1394error_t
_dc1394_video_iso_handler(raw1394handle_t handle, int channel, size_t length, uint32_t *data) 
{

    /* the first packet of a frame has a 1 in the lsb of the header */
#ifdef LIBRAW1394_OLD
  if ( (data[0] & 0x1) && (_dc1394_frame_captured[channel] != 1) ) {
#else
  if ( (ntohl(data[0]) & 0x00000001UL) && (_dc1394_frame_captured[channel] != 1) ) {
#endif
    _dc1394_offset[channel]= 0;
    _dc1394_frame_captured[channel]= 2;
    
    /* the plus 1 is to shift past the first header quadlet*/
    memcpy((char*)_dc1394_buffer[channel], (char*)(data+1),
	   4*_dc1394_quadlets_per_packet[channel]);
    _dc1394_offset[channel]+=_dc1394_quadlets_per_packet[channel];
    
  }
  else if (_dc1394_frame_captured[channel] == 2) {
    int copy_n_quadlets = _dc1394_quadlets_per_packet[channel];
  
    if( _dc1394_offset[channel] + copy_n_quadlets
	> _dc1394_quadlets_per_frame[channel]) {
      /* this is the last packet. Maybe, we just need a part of its data */
      copy_n_quadlets= _dc1394_quadlets_per_frame[channel]
	- _dc1394_offset[channel];
    }
    
    memcpy((char*)(_dc1394_buffer[channel]+_dc1394_offset[channel]),
	   (char*)(data+1), 4*copy_n_quadlets);
    _dc1394_offset[channel]+= copy_n_quadlets;
    
    if (_dc1394_offset[channel] == _dc1394_quadlets_per_frame[channel]) {
      _dc1394_frame_captured[channel]= 1;
      _dc1394_all_captured--;
      _dc1394_offset[channel]= 0;
      
    }
    
  }
  
  return DC1394_SUCCESS;
}

dc1394error_t
_dc1394_open_dma_device(dc1394camera_t *camera)
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);
  char filename[64];
  struct stat statbuf;

  // if the file has already been opened: increment the number of uses and return
  if (_dc1394_num_using_fd[camera->port] != 0) {
    _dc1394_num_using_fd[camera->port]++;
    craw->capture.dma_fd = _dc1394_dma_fd[camera->port];
    return DC1394_SUCCESS;
  }

  // if the dma device file has been set manually, use that device name
  if (craw->capture.dma_device_file!=NULL) {
    if ( (craw->capture.dma_fd = open(craw->capture.dma_device_file,O_RDONLY)) < 0 ) {
      return DC1394_INVALID_VIDEO1394_DEVICE;
    }
    else {
      _dc1394_dma_fd[camera->port] = craw->capture.dma_fd;
      _dc1394_num_using_fd[camera->port]++;
      return DC1394_SUCCESS;
    }
  }

  // automatic mode: try to open several usual device files.
  sprintf(filename,"/dev/video1394/%d",camera->port);
  if ( stat (filename, &statbuf) == 0 &&
          S_ISCHR (statbuf.st_mode) &&
          (craw->capture.dma_fd = open(filename,O_RDONLY)) >= 0 ) {
    _dc1394_dma_fd[camera->port] = craw->capture.dma_fd;
    _dc1394_num_using_fd[camera->port]++;
    return DC1394_SUCCESS;
  }

  sprintf(filename,"/dev/video1394-%d",camera->port);
  if ( stat (filename, &statbuf) == 0 &&
          S_ISCHR (statbuf.st_mode) &&
          (craw->capture.dma_fd = open(filename,O_RDONLY)) >= 0 ) {
    _dc1394_dma_fd[camera->port] = craw->capture.dma_fd;
    _dc1394_num_using_fd[camera->port]++;
    return DC1394_SUCCESS;
  }
  
  if (camera->port==0) {
    sprintf(filename,"/dev/video1394");
    if ( stat (filename, &statbuf) == 0 &&
            S_ISCHR (statbuf.st_mode) &&
            (craw->capture.dma_fd = open(filename,O_RDONLY)) >= 0 ) {
      _dc1394_dma_fd[camera->port] = craw->capture.dma_fd;
      _dc1394_num_using_fd[camera->port]++;
      return DC1394_SUCCESS;
    }
  }

  return DC1394_FAILURE;
}

/*****************************************************
 _ dc1394_dma_basic_setup

 This sets up the dma for the given camera. 
 _dc1394_capture_basic_setup must be called before

******************************************************/
dc1394error_t
_dc1394_capture_dma_setup(dc1394camera_t *camera, uint32_t num_dma_buffers)
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);
  struct video1394_mmap vmmap;
  struct video1394_wait vwait;
  uint32_t i;
  dc1394video_frame_t * f;

  /* using_fd counter array NULL if not used yet -- initialize */
  if ( _dc1394_num_using_fd == NULL ) {
    _dc1394_num_using_fd = calloc( MAX_NUM_PORTS, sizeof(uint32_t) );
    _dc1394_dma_fd = calloc( MAX_NUM_PORTS, sizeof(uint32_t) );
  }

  if (_dc1394_open_dma_device(camera) != DC1394_SUCCESS) {
    fprintf (stderr, "Could not open video1394 device file in /dev\n");
    return DC1394_INVALID_VIDEO1394_DEVICE;
  }
  
  vmmap.sync_tag= 1;
  vmmap.nb_buffers= num_dma_buffers;
  vmmap.flags= VIDEO1394_SYNC_FRAMES;
  vmmap.buf_size= craw->capture.frames[0].total_bytes; //number of bytes needed
  vmmap.channel= camera->iso_channel;
  
  /* tell the video1394 system that we want to listen to the given channel */
  if (ioctl(craw->capture.dma_fd, VIDEO1394_IOC_LISTEN_CHANNEL, &vmmap) < 0) {
    printf("(%s) VIDEO1394_IOC_LISTEN_CHANNEL ioctl failed: %s\n", __FILE__,
            strerror (errno));
    return DC1394_IOCTL_FAILURE;
  }
  // starting from here we use the ISO channel so we set the flag in the camera struct:
  camera->capture_is_set=2;

  //fprintf(stderr,"listening channel set\n");
  
  craw->capture.dma_frame_size= vmmap.buf_size;
  craw->capture.num_dma_buffers= vmmap.nb_buffers;
  craw->capture.dma_last_buffer= -1;
  vwait.channel= camera->iso_channel;
  
  /* QUEUE the buffers */
  for (i= 0; i < vmmap.nb_buffers; i++) {
    vwait.buffer= i;
    
    if (ioctl(craw->capture.dma_fd,VIDEO1394_IOC_LISTEN_QUEUE_BUFFER,&vwait) < 0) {
      printf("(%s) VIDEO1394_IOC_LISTEN_QUEUE_BUFFER ioctl failed!\n", __FILE__);
      ioctl(craw->capture.dma_fd, VIDEO1394_IOC_UNLISTEN_CHANNEL, &(vwait.channel));
      camera->capture_is_set=0;
      return DC1394_IOCTL_FAILURE;
    }
    
  }
  
  craw->capture.dma_ring_buffer= mmap(0, vmmap.nb_buffers * vmmap.buf_size,
				 PROT_READ,MAP_SHARED, craw->capture.dma_fd, 0);
  
  /* make sure the ring buffer was allocated */
  if (craw->capture.dma_ring_buffer == (uint8_t*)(-1)) {
    printf("(%s) mmap failed!\n", __FILE__);
    ioctl(craw->capture.dma_fd, VIDEO1394_IOC_UNLISTEN_CHANNEL, &vmmap.channel);
    camera->capture_is_set=0;
    return DC1394_IOCTL_FAILURE;
  }
  
  craw->capture.dma_buffer_size= vmmap.buf_size * vmmap.nb_buffers;

  for (i = 0; i < num_dma_buffers; i++) {
    f = craw->capture.frames + i;
    if (i > 0)
      memcpy (f, craw->capture.frames, sizeof (dc1394video_frame_t));
    f->image = (unsigned char *)(craw->capture.dma_ring_buffer +
        i * craw->capture.dma_frame_size);
    f->id = i;
  }

  //fprintf(stderr,"num dma buffers in setup: %d\n",craw->capture.num_dma_buffers);
  return DC1394_SUCCESS;
}


/* This function allows you to specify the DMA device filename manually. */
dc1394error_t
dc1394_capture_set_dma_device_filename(dc1394camera_t* camera, char *filename)
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);
  if (craw->capture.dma_device_file==NULL) {
    craw->capture.dma_device_file=(char*)malloc(64*sizeof(char));
    if (craw->capture.dma_device_file==NULL)
      return DC1394_MEMORY_ALLOCATION_FAILURE;
  }
  // note that the device filename is limited to 64 characters.
  memcpy(craw->capture.dma_device_file,filename,64*sizeof(char));

  return DC1394_SUCCESS;
}

/*************************************************************
 CAPTURE SETUP
**************************************************************/
dc1394error_t 
dc1394_capture_setup(dc1394camera_t *camera) 
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);
  dc1394error_t err;

  // we first have to verify that channels/bandwidth have been allocated.
  err=dc1394_allocate_iso_channel_and_bandwidth(camera);
  DC1394_ERR_RTN(err,"Could not allocate ISO channel and bandwidth!");

  // TODO: check if we already have a valid handle before allocating a new one...
  craw->capture.handle=raw1394_new_handle();
  raw1394_set_port(craw->capture.handle,camera->port);

  craw->capture.frames = malloc (sizeof (dc1394video_frame_t));

  err=_dc1394_capture_basic_setup(camera, craw->capture.frames);
  DC1394_ERR_RTN(err,"Could not setup capture");
  /*
  fprintf(stderr,"qpf: %d qpp: %d size [%d x %d]\n",
	  craw->capture.quadlets_per_frame, craw->capture.quadlets_per_packet,
	  craw->capture.frame_width, craw->capture.frame_height);
  */
  craw->capture.capture_buffer=
    (uint8_t*)malloc(craw->capture.frames->total_bytes);
  
  if (craw->capture.capture_buffer == NULL) {
    printf("(%s) unable to allocate memory for capture buffer\n", __FILE__);
    return DC1394_MEMORY_ALLOCATION_FAILURE;
  }
  else
    camera->capture_is_set=1;

  return err;
}

dc1394error_t
dc1394_capture_setup_dma(dc1394camera_t *camera, uint32_t num_dma_buffers)
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);
  dc1394error_t err;

  // we first have to verify that channels/bandwidth have been allocated.
  err=dc1394_allocate_iso_channel_and_bandwidth(camera);
  DC1394_ERR_RTN(err,"Could not allocate ISO channel and bandwidth!");

  craw->capture.frames = malloc (num_dma_buffers * sizeof (dc1394video_frame_t));

  err=_dc1394_capture_basic_setup(camera, craw->capture.frames);
  if (err != DC1394_SUCCESS) {
    dc1394_free_iso_channel_and_bandwidth(camera);
    free (craw->capture.frames);
    craw->capture.frames = NULL;
  }
  DC1394_ERR_RTN(err,"Could not setup capture");
  
  // the capture_is_set flag is set inside this function:
  err=_dc1394_capture_dma_setup (camera, num_dma_buffers);
  if (err != DC1394_SUCCESS) {
    dc1394_free_iso_channel_and_bandwidth(camera);
    free (craw->capture.frames);
    craw->capture.frames = NULL;
  }
  DC1394_ERR_RTN(err,"Could not setup DMA capture");

  return err;
}

/*****************************************************
 CAPTURE_STOP
*****************************************************/

dc1394error_t 
dc1394_capture_stop(dc1394camera_t *camera) 
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);

  if (camera->capture_is_set>0) {
    switch (camera->capture_is_set) {
      case 1: // RAW 1394
        if (craw->capture.capture_buffer != NULL) {
          free(craw->capture.capture_buffer);
          craw->capture.capture_buffer=NULL;
        }
        free (craw->capture.frames);
        craw->capture.frames = NULL;
        break;
      case 2: // DMA (VIDEO1394)
        // unlisten
        if (ioctl(craw->capture.dma_fd, VIDEO1394_IOC_UNLISTEN_CHANNEL,
              &(camera->iso_channel)) < 0) 
          return DC1394_IOCTL_FAILURE;

        // release
        if (craw->capture.dma_ring_buffer) {
          munmap((void*)craw->capture.dma_ring_buffer,craw->capture.dma_buffer_size);
        }

        _dc1394_num_using_fd[camera->port]--;

        if (_dc1394_num_using_fd[camera->port] == 0) {

          while (close(craw->capture.dma_fd) != 0) {
            printf("(%s) waiting for dma_fd to close\n", __FILE__);
            sleep (1);
          }

        }
        free (craw->capture.frames);
        craw->capture.frames = NULL;

        // this dma_device file is allocated by the strdup() function and can be freed here without problems.
        free(craw->capture.dma_device_file);
        craw->capture.dma_device_file=NULL;
        break;
      default:
        return DC1394_INVALID_CAPTURE_MODE;
        break;
    }
    
    // capture is not set anymore
    camera->capture_is_set=0;
    dc1394_free_iso_channel_and_bandwidth(camera);
    
    // free the additional capture handle
    raw1394_destroy_handle(craw->capture.handle);
  }
  else {
    return DC1394_CAPTURE_IS_NOT_SET;
  }
  
  return DC1394_SUCCESS;
}

/****************************************************
 CAPTURE
*****************************************************/
dc1394error_t 
dc1394_capture(dc1394camera_t **cams, uint32_t num) 
{
  int i, j;
  _dc1394_all_captured= num;
  dc1394capture_t * captures[num];

  /*
    this first routine does the setup-
    sets up the global variables needed in the handler,
    sets the iso handler,
    tells the 1394 subsystem to listen for iso packets
  */
  for (i = 0; i < num; i++) {
    DC1394_CAST_CAMERA_TO_LINUX(craw, cams[i]);
    captures[i] = &(craw->capture);
  }
  for (i= 0; i < num; i++)  {
    // we are about to use the iso channels, so set the capture flag
    // cams[i]->capture_is_set=1;

    _dc1394_buffer[cams[i]->iso_channel]= (int*)captures[i]->capture_buffer;
    
    if (raw1394_set_iso_handler(captures[i]->handle, cams[i]->iso_channel, _dc1394_video_iso_handler) < 0)  {
      /* error handling- for some reason something didn't work, 
	 so we have to reset everything....*/
      printf("(%s:%d) error!\n",__FILE__, __LINE__);
      
      for (j= i - 1; j > -1; j--)  {
	raw1394_stop_iso_rcv(captures[i]->handle, cams[j]->iso_channel);
	raw1394_set_iso_handler(captures[i]->handle, cams[j]->iso_channel, NULL);
      }
      
      return DC1394_RAW1394_CAPTURE_FAILURE;
    }

    //fprintf(stderr,"handle: 0x%x\n",(unsigned int)cams[i]->handle);
    _dc1394_frame_captured[cams[i]->iso_channel] = 0;
    _dc1394_quadlets_per_frame[cams[i]->iso_channel] = captures[i]->frames->total_bytes / 4;
    _dc1394_quadlets_per_packet[cams[i]->iso_channel] = captures[i]->frames->bytes_per_packet / 4;

    //fprintf(stderr,"starting reception...\n");
    //fprintf(stderr,"handle: 0x%x\n",(unsigned int)cams[i]->handle);
    if (raw1394_start_iso_rcv(captures[i]->handle,cams[i]->iso_channel) < 0)  {
      /* error handling- for some reason something didn't work, 
	 so we have to reset everything....*/
      fprintf(stderr,"(%s:%d) error!\n", __FILE__, __LINE__);
      
      for (j= 0; j < num; j++)  {
	raw1394_stop_iso_rcv(captures[i]->handle, cams[j]->iso_channel);
	raw1394_set_iso_handler(captures[i]->handle, cams[j]->iso_channel, NULL);
      }
      return DC1394_RAW1394_CAPTURE_FAILURE;
    }
    
  }
  //fprintf(stderr,"data loop...\n");

  /* now we iterate till the data is here*/
  while (_dc1394_all_captured != 0)  {
    //fprintf(stderr,"iter ");
    raw1394_loop_iterate(captures[0]->handle);
  }
  
  //fprintf(stderr,"data loop...\n");
  /* now stop the subsystem from listening*/
  for (i= 0; i < num; i++)  {
    raw1394_stop_iso_rcv(captures[i]->handle, cams[i]->iso_channel);
    raw1394_set_iso_handler(captures[i]->handle,cams[i]->iso_channel, NULL);

    // we are not listening anymore, release the capture flags
    // cams[i]->capture_is_set=0;
  }
  
  return DC1394_SUCCESS;
}

uint8_t*
dc1394_capture_get_buffer(dc1394camera_t *camera)
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);
  return craw->capture.capture_buffer;
}

dc1394video_frame_t *
dc1394_capture_dequeue_dma (dc1394camera_t * camera, dc1394video_policy_t policy)
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);
  dc1394capture_t * capture = &(craw->capture);
  dc1394video_frame_t * frame;
  struct video1394_wait vwait;
  int cb;
  int result=-1;

  cb = (capture->dma_last_buffer + 1) % capture->num_dma_buffers;
  frame = capture->frames + cb;

  vwait.channel = camera->iso_channel;
  vwait.buffer = cb;
  switch (policy) {
    case DC1394_VIDEO1394_POLL:
      result=ioctl(capture->dma_fd, VIDEO1394_IOC_LISTEN_POLL_BUFFER, &vwait);
      break;
    case DC1394_VIDEO1394_WAIT:
    default:
      result=ioctl(capture->dma_fd, VIDEO1394_IOC_LISTEN_WAIT_BUFFER, &vwait);
      break;
  }
  if ( result != 0) {       
    if ((policy==DC1394_VIDEO1394_POLL) && (errno == EINTR)) {                       
      // when no frames is present, say so.
      return NULL;
    }
    else {
      printf("(%s) VIDEO1394_IOC_LISTEN_WAIT/POLL_BUFFER ioctl failed!\n", __FILE__);
      return NULL;
    }
  }

  capture->dma_last_buffer = cb;

  frame->frames_behind = vwait.buffer;
  frame->timestamp = (uint64_t) vwait.filltime.tv_sec * 1000000 +
    vwait.filltime.tv_usec;

  return frame;
}

dc1394error_t
dc1394_capture_enqueue_dma (dc1394camera_t * camera, dc1394video_frame_t * frame)
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);
  struct video1394_wait vwait;

  if (frame->camera != camera) {
    printf ("(%s) dc1394_capture_enqueue_dma: camera does not match frame's camera\n",
        __FILE__);
    return DC1394_INVALID_ARGUMENT_VALUE;
  }
  
  vwait.channel = camera->iso_channel;
  vwait.buffer = frame->id;
  
  if (ioctl(craw->capture.dma_fd, VIDEO1394_IOC_LISTEN_QUEUE_BUFFER, &vwait) < 0)  {
    printf("(%s) VIDEO1394_IOC_LISTEN_QUEUE_BUFFER failed in done with buffer!\n", __FILE__);
    return DC1394_IOCTL_FAILURE;
  }
  
  return DC1394_SUCCESS;
}

int
dc1394_capture_get_dma_fd (dc1394camera_t * camera)
{
  DC1394_CAST_CAMERA_TO_LINUX(craw, camera);
  return craw->capture.dma_fd;
}
