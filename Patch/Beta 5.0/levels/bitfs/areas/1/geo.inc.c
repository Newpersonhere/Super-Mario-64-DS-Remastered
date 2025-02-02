// 0x0E0007A0
const GeoLayout bitfs_geo_0007A0[] = {
   GEO_NODE_SCREEN_AREA(10, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, SCREEN_HEIGHT/2),
   GEO_OPEN_NODE(),
      GEO_ZBUFFER(0),
      GEO_OPEN_NODE(),
         GEO_NODE_ORTHO(100),
         GEO_OPEN_NODE(),
            GEO_BACKGROUND(BACKGROUND_FLAMING_SKY, geo_skybox_main),
         GEO_CLOSE_NODE(),
      GEO_CLOSE_NODE(),
      GEO_ZBUFFER(1),
      GEO_OPEN_NODE(),
         GEO_CAMERA_FRUSTUM_WITH_FUNC(20, 100, 20000, geo_camera_fov),
         GEO_OPEN_NODE(),
            GEO_CAMERA(14, 0, 2000, 6000, 0, -4500, -8000, geo_camera_main),
            GEO_OPEN_NODE(),
               GEO_DISPLAY_LIST(LAYER_ALPHA, bitfs_seg7_dl_07002118),
               GEO_ASM(   0, geo_movtex_pause_control),
               GEO_ASM(0x1901, geo_movtex_draw_nocolor),
               GEO_ASM(0x1902, geo_movtex_draw_nocolor),
               GEO_ASM(0x1903, geo_movtex_draw_nocolor),
               GEO_RENDER_OBJ(),
               GEO_ASM(  12, geo_envfx_main),
            GEO_CLOSE_NODE(),
         GEO_CLOSE_NODE(),
      GEO_CLOSE_NODE(),
   GEO_CLOSE_NODE(),
   GEO_END(),
};
