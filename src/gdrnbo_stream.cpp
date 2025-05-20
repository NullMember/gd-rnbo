#include "gdrnbo_stream.h"

namespace godot {

    // RNBOStream class implementation
    void RNBOStream::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("instantiate_playback"), &RNBOStream::_instantiate_playback);
        ClassDB::bind_method(D_METHOD("get_length"), &RNBOStream::_get_length);
        ClassDB::bind_method(D_METHOD("is_monophonic"), &RNBOStream::_is_monophonic);
        ClassDB::bind_method(D_METHOD("get_bpm"), &RNBOStream::_get_bpm);
        ClassDB::bind_method(D_METHOD("get_beat_count"), &RNBOStream::_get_beat_count);
        ClassDB::bind_method(D_METHOD("get_parameter_list"), &RNBOStream::_get_parameter_list);
    }
    RNBOStream::RNBOStream()
    {
        rnbo_object = new RNBO::CoreObject();
        rnbo_object->initialize();

        audio_server = AudioServer::get_singleton();
        instance = nullptr;
        properties = nullptr;
        
        num_in_channels = rnbo_object->getNumInputChannels();
        num_out_channels = rnbo_object->getNumOutputChannels();
        num_messages = rnbo_object->getNumMessages();
        num_params = rnbo_object->getNumParameters();
        num_signal_in_params = rnbo_object->getNumSignalInParameters();
        num_signal_out_params = rnbo_object->getNumSignalOutParameters();
        
        param_info = new RNBO::ParameterInfo*[num_params];
        for (RNBO::ParameterIndex i = 0; i < num_params; i++) {
            param_info[i] = new RNBO::ParameterInfo();
            rnbo_object->getParameterInfo(i, param_info[i]);
            if (String(rnbo_object->getParameterId(i)) == "play") {
                rnbo_play_index = i;
                rnbo_has_play = true;
            } else if (String(rnbo_object->getParameterId(i)) == "stop") {
                rnbo_stop_index = i;
                rnbo_has_stop = true;
            }
        }
    }
    RNBOStream::~RNBOStream()
    {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
        if (properties) {
            delete properties;
            properties = nullptr;
        }
        if (rnbo_object) {
            delete rnbo_object;
            rnbo_object = nullptr;
        }
        if (param_info) {
            for (RNBO::ParameterIndex i = 0; i < num_params; i++) {
                delete param_info[i];
            }
            delete[] param_info;
            param_info = nullptr;
        }
    }
    Ref<AudioStreamPlayback> RNBOStream::_instantiate_playback() const
    {
        Ref<RNBOPlayback> playback;
        playback.instantiate();
        playback->rnbo_object = rnbo_object;
        playback->base = const_cast<RNBOStream *>(this);
        playback->rnbo_has_play = rnbo_has_play;
        playback->rnbo_has_stop = rnbo_has_stop;
        playback->rnbo_play_index = rnbo_play_index;
        playback->rnbo_stop_index = rnbo_stop_index;
        return playback;
    }
    String RNBOStream::_get_stream_name() const
    {
        return "RNBO Stream";
    }
    double RNBOStream::_get_length() const
    {
        return 0.0; // No length for now
    }
    bool RNBOStream::_is_monophonic() const
    {
        return false; // Not monophonic
    }
    double RNBOStream::_get_bpm() const
    {
        return 120.0; // Default BPM
    }
    int32_t RNBOStream::_get_beat_count() const
    {
        return 4; // Default beat count
    }
    TypedArray<Dictionary> RNBOStream::_get_parameter_list() const
    {
        TypedArray<Dictionary> param_list;
        // length parameter
        Dictionary param_length;
        param_length["name"] = String("length");
        param_length["value"] = 0.0;
        param_length["type"] = Variant::FLOAT;
        param_length["hint"] = PROPERTY_HINT_RANGE;
        param_length["hint_string"] = String("0.0,100.0");
        param_length["usage"] = PROPERTY_USAGE_DEFAULT;
        param_length["default_value"] = 0.0;
        param_list.push_back(param_length);
        for (RNBO::ParameterIndex i = 0; i < num_params; i++) {
            Dictionary param;
            param["name"] = String(rnbo_object->getParameterId(i));
            param["value"] = rnbo_object->getParameterValue(i);
            param["type"] = Variant::FLOAT; // Assuming all parameters are float for now
            param["hint"] = PROPERTY_HINT_RANGE;
            param["hint_string"] = String::num(param_info[i]->min) + "," + String::num(param_info[i]->max);
            param["usage"] = PROPERTY_USAGE_DEFAULT;
            param["default_value"] = param_info[i]->initialValue;
            if (!(param["name"] == String("play") | param["name"] == String("stop")))
            {
                param_list.push_back(param);
            }
        }
        return param_list;
    }

    // RNBOPlayback class implementation
    void RNBOPlayback::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("start", "from_pos"), &RNBOPlayback::_start, Variant::FLOAT);
        ClassDB::bind_method(D_METHOD("stop"), &RNBOPlayback::_stop);
        ClassDB::bind_method(D_METHOD("is_playing"), &RNBOPlayback::_is_playing);
        ClassDB::bind_method(D_METHOD("get_loop_count"), &RNBOPlayback::_get_loop_count);
        ClassDB::bind_method(D_METHOD("get_playback_position"), &RNBOPlayback::_get_playback_position);
        ClassDB::bind_method(D_METHOD("seek", "position"), &RNBOPlayback::_seek, Variant::FLOAT);
        ClassDB::bind_method(D_METHOD("tag_used_streams"), &RNBOPlayback::_tag_used_streams);
        ClassDB::bind_method(D_METHOD("set_parameter", "name", "value"), &RNBOPlayback::_set_parameter, Variant::STRING, Variant::OBJECT);
        ClassDB::bind_method(D_METHOD("get_parameter", "name"), &RNBOPlayback::_get_parameter, Variant::STRING);
    }
    RNBOPlayback::RNBOPlayback()
    {
        audio_server = AudioServer::get_singleton();

        in_audio = new double*[2];
        out_audio = new double*[2];
        for (RNBO::Index i = 0; i < 2; i++) {
            in_audio[i] = new double[audio_server->get_mix_rate()];
            out_audio[i] = new double[audio_server->get_mix_rate()];
        }
        // Initialize the audio buffers
        for (RNBO::Index i = 0; i < 2; i++) {
            for (RNBO::Index j = 0; j < audio_server->get_mix_rate(); j++) {
                in_audio[i][j] = 0.0;
                out_audio[i][j] = 0.0;
            }
        }
    }

    RNBOPlayback::~RNBOPlayback()
    {
        if (in_audio) {
            for (RNBO::Index i = 0; i < 2; i++) {
                delete[] in_audio[i];
            }
            delete[] in_audio;
            in_audio = nullptr;
        }
        if (out_audio) {
            for (RNBO::Index i = 0; i < 2; i++) {
                delete[] out_audio[i];
            }
            delete[] out_audio;
            out_audio = nullptr;
        }
    }
    void RNBOPlayback::_start(double p_from_pos)
    {
        // Start the playback
        is_playing = true;
        playback_position = p_from_pos;
        if(rnbo_has_play){
            rnbo_object->setParameterValue(rnbo_play_index, 1.0);
        }
    }

    void RNBOPlayback::_stop()
    {
        // Stop the playback
        is_playing = false;
        playback_position = 0.0;
    }
    bool RNBOPlayback::_is_playing() const
    {
        return is_playing;
    }
    int32_t RNBOPlayback::_get_loop_count() const
    {
        return 0; // No looping for now
    }
    double RNBOPlayback::_get_playback_position() const
    {
        return playback_position;
    }
    void RNBOPlayback::_seek(double p_position)
    {
        // Seek to the specified position
        playback_position = p_position;
    }
    int32_t RNBOPlayback::_mix(AudioFrame *p_buffer, float p_rate_scale, int32_t p_frames)
    {
        if (rnbo_has_stop) {
            auto stop = (rnbo_object->getParameterValue(rnbo_stop_index) > 0.0);
            if(stop) {
                rnbo_object->setParameterValue(rnbo_stop_index, 0.0);
                if (is_playing) {
                    playback_position = 0.0;
                    is_playing = false;
                }
            }
        }
        if (playback_length > 0.0 & playback_position >= playback_length & is_playing) {
            playback_position = 0.0;
            is_playing = false;
        }
        // Mix the audio
        if (rnbo_object) {
            if (is_playing) {
                while(!rnbo_object->prepareToProcess(audio_server->get_mix_rate(), p_frames)) {
                    // Wait for the object to be ready
                };

                rnbo_object->process(in_audio, 2, out_audio, 2, p_frames);

                for (size_t i = 0; i < p_frames; i++)
                {
                    p_buffer[i].left = out_audio[0][i];
                    p_buffer[i].right = out_audio[1][i];
                }
                playback_position += p_frames / audio_server->get_mix_rate();
                return p_frames;
            }
        }
        return 0;
    }
    void RNBOPlayback::_tag_used_streams()
    {
        return; // No streams to tag for now
    }
    void RNBOPlayback::_set_parameter(const StringName &p_name, const Variant &p_value)
    {
        // Set the parameter value
        if(p_name == String("length")) {
            playback_length = p_value;
            return;
        }
        for (RNBO::ParameterIndex i = 0; i < rnbo_object->getNumParameters(); i++) {
            if (String(rnbo_object->getParameterId(i)) == p_name) {
                rnbo_object->setParameterValue(i, p_value);
                break;
            }
        }
    }
    Variant RNBOPlayback::_get_parameter(const StringName &p_name) const
    {
        // Get the parameter value
        if (p_name == String("length"))
        {
            return playback_length;
        }
        for (RNBO::ParameterIndex i = 0; i < rnbo_object->getNumParameters(); i++) {
            if (String(rnbo_object->getParameterId(i)) == p_name) {
                return rnbo_object->getParameterValue(i);
            }
        }
        return Variant();
    }
}