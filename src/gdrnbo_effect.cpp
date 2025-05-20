#include "gdrnbo_effect.h"

namespace godot {

    // GDRNBO Effect Instance

    void RNBOInstance::_bind_methods() {
        // ClassDB::bind_method(D_METHOD("process"), &RNBOInstance::_process);
    }

    void RNBOInstance::_process(const void *p_src_buffer, AudioFrame *p_dst_buffer, int32_t p_frame_count)
    {
        if (rnbo_object) {
            AudioFrame *src_buffer = (AudioFrame *)p_src_buffer;
            for (size_t i = 0; i < p_frame_count; i++)
            {
                in_audio[0][i] = src_buffer[i].left;
                in_audio[1][i] = src_buffer[i].right;
            }
            
            while(!rnbo_object->prepareToProcess(audio_server->get_mix_rate(), p_frame_count)) {
                // Wait for the object to be ready
            };

            rnbo_object->process(in_audio, 2, out_audio, 2, p_frame_count);

            for (size_t i = 0; i < p_frame_count; i++)
            {
                p_dst_buffer[i].left = out_audio[0][i];
                p_dst_buffer[i].right = out_audio[1][i];
            }
        }
    }

    RNBOInstance::RNBOInstance()
    {
        audio_server = AudioServer::get_singleton();
        rnbo_object = new RNBO::CoreObject();
        
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
        }

        in_audio = new double*[2];
        out_audio = new double*[2];
        for (RNBO::Index i = 0; i < 2; i++) {
            in_audio[i] = new double[audio_server->get_mix_rate()];
            out_audio[i] = new double[audio_server->get_mix_rate()];
        }
    }

    RNBOInstance::~RNBOInstance()
    {
        if (rnbo_object) {
            delete rnbo_object;
            rnbo_object = nullptr;
        }
        if (in_audio) {
            for (RNBO::Index i = 0; i < num_in_channels; i++) {
                delete[] in_audio[i];
            }
            delete[] in_audio;
            in_audio = nullptr;
        }
        if (out_audio) {
            for (RNBO::Index i = 0; i < num_out_channels; i++) {
                delete[] out_audio[i];
            }
            delete[] out_audio;
            out_audio = nullptr;
        }
        if (param_info) {
            for (RNBO::ParameterIndex i = 0; i < num_params; i++) {
                delete param_info[i];
            }
            delete[] param_info;
            param_info = nullptr;
        }
    }

    // GDRNBO Effect

    void RNBOEffect::_bind_methods() {
        ClassDB::bind_method(D_METHOD("instantiate"), &RNBOEffect::_instantiate);
    }
    RNBOEffect::RNBOEffect() {
        audio_server = AudioServer::get_singleton();
    }
    RNBOEffect::~RNBOEffect() {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }
    Ref<AudioEffectInstance> RNBOEffect::_instantiate() {
        Ref<RNBOInstance> instance;
        instance.instantiate();
        instance->base = Ref<RNBOEffect>(this);
        this->instance = instance.ptr();
        // set parameters
        return instance;
    }

    void RNBOEffect::_get_property_list(List<PropertyInfo> *r_props) const {
        // Add properties to the list
        if(instance) {
            for (size_t i = 0; i < instance->num_params; i++)
            {
                r_props->push_back(PropertyInfo(Variant::FLOAT, String(instance->rnbo_object->getParameterId(i)), PROPERTY_HINT_RANGE, String::num(instance->param_info[i]->min) + "," + String::num(instance->param_info[i]->max), PROPERTY_USAGE_DEFAULT));
            }
        }
    }

    bool RNBOEffect::_get(const StringName &p_property, Variant &r_value) const
    {
        if(instance) {
            for (size_t i = 0; i < instance->num_params; i++)
            {
                if (p_property == String(instance->rnbo_object->getParameterId(i))) {
                    r_value = instance->rnbo_object->getParameterValue(i);
                    return true;
                }
            }
        }
        return false;
    }
    bool RNBOEffect::_set(const StringName &p_property, const Variant &p_value)
    {
        if(instance) {
            for (size_t i = 0; i < instance->num_params; i++)
            {
                if (p_property == String(instance->rnbo_object->getParameterId(i))) {
                    instance->rnbo_object->setParameterValue(i, p_value);
                    return true;
                }
            }
        }
        return false;
    }
}