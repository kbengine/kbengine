material test_material
{
    technique default
    {
    }
}

// Empty sky w/o components. Happens to be very dark.
caelum_sky_system EmptySkyScript
{
}

// Abstract base for other things defined here.
// These defaults should be mostly the same as in autoConfigure
abstract caelum_sky_system DefaultBase
{
    // J2000
    julian_day 2451545.0
    time_scale 1

    point_starfield {
        magnitude_scale 2.51189
        mag0_pixel_size 16
        min_pixel_size 4
        max_pixel_size 6
    }

    manage_ambient_light true
    minimum_ambient_light 0.1 0.1 0.3

    manage_scene_fog yes
    ground_fog_density_multiplier 0.03

    sun {
        ambient_multiplier 0.5 0.5 0.5
        diffuse_multiplier 3 3 2.7
        specular_multiplier 5 5 5

        auto_disable_threshold 0.05
        auto_disable true
    }

    moon {
        ambient_multiplier 0.2 0.2 0.2
        diffuse_multiplier 1 1 .9
        specular_multiplier 1 1 1

        auto_disable_threshold 0.05
        auto_disable true
    }

    // Off by default
    /*
    depth_composer {
        debug_depth_render off
        haze_enabled no
        ground_fog_enabled no
        ground_fog_vertical_decay 0.06
        ground_fog_base_level 0
    }
    */

    sky_dome {
        haze_enabled no
        sky_gradients_image EarthClearSky2.png
        atmosphere_depth_image AtmosphereDepth.png
    }
}

// Default sky; this is what you get from CaelumSystem::autoConfigure
// DefaultBase doesn't contain clouds because they're impossible to remove when overriding.
caelum_sky_system DefaultSky: DefaultBase
{
    cloud_system
    {
        cloud_layer
        {
            height 3000
            coverage 0.3
        }
    }
}

// Test overriding stars to make them big and puffy.
caelum_sky_system BigPuffyStars: DefaultBase
{
    // Midnight before J2000
    julian_day 2451544.5

    point_starfield
    {
        magnitude_scale 2.51189
        mag0_pixel_size 100
        min_pixel_size 16
        max_pixel_size 16
    }
}

// Make sure the fog composer doesn't affect stars.
caelum_sky_system BigPuffyStarsWithFogComposer: BigPuffyStars
{
    depth_composer
    {
        debug_depth_render off
        haze_enabled yes
        ground_fog_enabled on
    }
}

// Override observer position for summer midnight at very high longitude.
// The sun remains visible all day.
// It will circle the sky at various altitudes but not fall beneath the horizon.
// If you wait long enough winter will come and the sun will never rise.
caelum_sky_system MidnightSun: DefaultBase
{
    julian_day 2451724.5
    time_scale 10000
    longitude 170.43
    latitude 86 0 2
}

// There should be an eclipse here; but nothing shows up.
caelum_sky_system Eclipse: DefaultBase
{
    julian_day 2451401.8
    latitude 45 6 17
    longitude 24 22 21
}

caelum_sky_system FogScriptTest: DefaultBase
{
    depth_composer
    {
        debug_depth_render off
        haze_enabled yes
        ground_fog_enabled on
    }
}

// Shows rain falling at an angle.
caelum_sky_system RainWindScriptTest: DefaultBase
{
    precipitation
    {
        intensity 0.6
        texture precipitation_rain.png

        // Wind speed. This is not available through the UI.
        wind_speed .5 0 .7

        // Slow the camera effect.
        camera_speed_scale 0.01 0.01 0.01
    }
}

// Shows rain falling up
caelum_sky_system RainDirectionScriptTest: RainWindScriptTest
{
    precipitation
    {
        falling_direction 0 1 0
        wind_speed 0 0 0
    }
}

// This is a good test with very visible distinct shadows for both sun and moon.
caelum_sky_system ShadowDebug: DefaultBase
{
    julian_day 2152102.878

    manage_ambient_light true
    minimum_ambient_light 0 0 0

    sun {
        ambient_multiplier 0 0 0
        diffuse_multiplier 10 0 0 
        specular_multiplier 0 0 0
    }

    moon {
        ambient_multiplier 0 0 0
        diffuse_multiplier 0 10 0
        specular_multiplier 0 0 0
    }
}

caelum_sky_system HugeAmbientFactor: DefaultBase
{
    julian_day 0.5

    minimum_ambient_light 2 2 2
}

caelum_sky_system BasicCloudScriptTest: DefaultBase
{
    cloud_system {
        cloud_layer FirstLow {
            height 300
            coverage 0.5
            cloud_uv_factor 500
        }

        cloud_layer SecondHigh {
            height 700
            coverage 0.5
            cloud_uv_factor 300
        }
    }
}

caelum_sky_system OverrideCloudScriptTest: BasicCloudScriptTest
{
    cloud_system {
        cloud_layer SecondHigh {
            coverage 0.7
        }

        cloud_layer FirstLow {
            height 560
        }

        cloud_layer {
            height 1000
            coverage 1
            blend_time 1
        }
    }
}

caelum_sky_system CloudMeshScriptTest: DefaultBase
{
    cloud_system {
        cloud_layer {
            height 300
            coverage 0.5
            cloud_uv_factor 3
            height_red_factor 10000

            // Mesh with a huge number of segments.
            mesh_width 5000
            mesh_height 5000
            mesh_width_segments 100
            mesh_height_segments 100
        }
    }
}

caelum_sky_system CloudFadeScriptTest: DefaultBase
{
    cloud_system {
        cloud_layer {
            height 300
            coverage 0.3
            cloud_uv_factor 3000

            near_fade_dist 500
            far_fade_dist 600
        }

        cloud_layer {
            height 500
            coverage 0.3
            cloud_uv_factor 3000

            near_fade_dist 500
            far_fade_dist 600
        }

        cloud_layer {
            height 700
            coverage 0.3
            cloud_uv_factor 3000

            near_fade_dist 500
            far_fade_dist 600
        }
    }
}

// This is not pretty; but still supported.
caelum_sky_system SkyDomeOverrideHazeTest: DefaultBase
{
    sky_dome
    {
        haze_enabled yes
    }
}

caelum_sky_system GroundFogNoise: DefaultBase
{
    depth_composer {
        debug_depth_render off
        haze_enabled no
        ground_fog_enabled yes
        ground_fog_vertical_decay 0.0000001
        ground_fog_base_level 50
    }
    ground_fog_density_multiplier 0.001
}

caelum_sky_system SandStormTest: DefaultBase
{
    global_fog_density_multiplier 0.1
    global_fog_colour_multiplier 1.3 0.6 0.2

    depth_composer {
        haze_enabled no
        ground_fog_enabled yes
        ground_fog_vertical_decay 0.01
        ground_fog_base_level 70
    }

    cloud_system {
        cloud_layer {
            height 500
            coverage 0.5
            cloud_speed 0.03 0.01
            cloud_uv_factor 1000
            near_fade_dist 10000
            far_fade_dist 30000
        }
        cloud_layer {
            height 600
            coverage 1
            cloud_speed -0.06 0.01
            cloud_uv_factor 1000
            near_fade_dist 10000
            far_fade_dist 30000
        }
    }
}
