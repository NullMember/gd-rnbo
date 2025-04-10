#pragma once

#ifndef GDRNBO_STREAM_H
#define GDRNBO_STREAM_H

#include <RNBO.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/audio_server.hpp>

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <type_traits>

namespace godot
{
    class GDRNBOEffect;

    // GDRNBO Effect Instance
    /**
     * @class GDRNBOInstance
     * @brief An instance of the GDRNBO effect.
     *
     * This class is responsible for processing audio and managing the RNBO object.
     **/
    class GDRNBOInstance : public AudioEffectInstance
    {
        GDCLASS(GDRNBOInstance, AudioEffectInstance);
        friend class GDRNBOEffect;
        
        private:
        RNBO::CoreObject *rnbo_object = nullptr;
        AudioServer *audio_server = nullptr;
        
        protected:
        double **in_audio = nullptr;
        double **out_audio = nullptr;
        
        RNBO::Index num_in_channels = 0;
        RNBO::Index num_out_channels = 0;
        RNBO::MessageIndex num_messages = 0;
        
        RNBO::ParameterIndex num_params = 0;
        RNBO::ParameterInfo **param_info = nullptr;
        
        RNBO::ParameterIndex num_signal_in_params = 0;
        RNBO::ParameterIndex num_signal_out_params = 0;
        static void _bind_methods();
        
        public:
        GDRNBOInstance();
        ~GDRNBOInstance();
        
        void _process(const void *p_src_buffer, AudioFrame *p_dst_buffer, int32_t p_frame_count) override;

        Ref<GDRNBOEffect> base;
    };

    // GDRNBO Effect
    /**
     * @class GDRNBOEffect
     * @brief The GDRNBO effect class.
     *
     * This class is responsible for creating instances of the GDRNBO effect.
     **/
    class GDRNBOEffect : public AudioEffect
    {
        GDCLASS(GDRNBOEffect, AudioEffect);
        friend class GDRNBOInstance;

    private:
        AudioServer *audio_server = nullptr;
        GDRNBOInstance *instance = nullptr;
        List<PropertyInfo> *properties = nullptr;
        
    protected:
        static void _bind_methods();

    public:
        Ref<AudioEffectInstance> _instantiate() override;
        GDRNBOEffect();
        ~GDRNBOEffect();

        void _get_property_list(List<PropertyInfo> *r_props) const;      // return list of properties
        bool _get(const StringName &p_property, Variant &r_value) const; // return true if property was found
        bool _set(const StringName &p_property, const Variant &p_value); // return true if property was found
    };
}

#endif