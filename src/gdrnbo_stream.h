#pragma once

#ifndef GDRNBO_STREAM_H
#define GDRNBO_STREAM_H

#include <RNBO.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/audio_server.hpp>

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <type_traits>

namespace godot
{

    class RNBOPlayback;

    class RNBOStream : public AudioStream
    {
        GDCLASS(RNBOStream, AudioStream);

    private:
        RNBO::CoreObject *rnbo_object = nullptr;
        AudioServer *audio_server = nullptr;
        RNBOPlayback *instance = nullptr;
        List<PropertyInfo> *properties = nullptr;
        
    protected:
        bool rnbo_has_play = false;
        bool rnbo_has_stop = false;
        RNBO::ParameterIndex rnbo_play_index = 0;
        RNBO::ParameterIndex rnbo_stop_index = 0;

        RNBO::Index num_in_channels = 0;
        RNBO::Index num_out_channels = 0;
        RNBO::MessageIndex num_messages = 0;
        
        RNBO::ParameterIndex num_params = 0;
        RNBO::ParameterInfo **param_info = nullptr;
        
        RNBO::ParameterIndex num_signal_in_params = 0;
        RNBO::ParameterIndex num_signal_out_params = 0;

        static void _bind_methods();

    public:
        RNBOStream();
        ~RNBOStream();

        void _process_event(const RNBO::ParameterEvent& event);

        Ref<AudioStreamPlayback> _instantiate_playback() const override;
        String _get_stream_name() const override;
        double _get_length() const override;
        bool _is_monophonic() const override;
        double _get_bpm() const override;
        int32_t _get_beat_count() const override;
        TypedArray<Dictionary> _get_parameter_list() const override;
    };

    class RNBOPlayback : public AudioStreamPlayback
    {
        GDCLASS(RNBOPlayback, AudioStreamPlayback);
        friend class RNBOStream;
        
        private:
            RNBO::CoreObject *rnbo_object = nullptr;
            AudioServer *audio_server = nullptr;
            RNBOStream *base = nullptr;
            
        protected:
            bool is_playing = false;
            double playback_position = 0.0;
            double playback_length = 2.0;
            bool rnbo_has_play = false;
            bool rnbo_has_stop = false;
            RNBO::ParameterIndex rnbo_play_index = 0;
            RNBO::ParameterIndex rnbo_stop_index = 0;

            double **in_audio = nullptr;
            double **out_audio = nullptr;
            
            static void _bind_methods();
        
        public:
            RNBOPlayback();
            ~RNBOPlayback();

            void _start(double p_from_pos) override;
            void _stop() override;
            bool _is_playing() const override;
            int32_t _get_loop_count() const override;
            double _get_playback_position() const override;
            void _seek(double p_position) override;
            int32_t _mix(AudioFrame *p_buffer, float p_rate_scale, int32_t p_frames) override;
            void _tag_used_streams() override;
            void _set_parameter(const StringName &p_name, const Variant &p_value) override;
            Variant _get_parameter(const StringName &p_name) const override;
    };
}

#endif