package com.example.demo.service;

import com.example.demo.dto.TelemetryRequestDTO;
import com.example.demo.model.TelemetryLog;
import com.example.demo.repository.TelemetryRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class TelemetryService {

    @Autowired
    private TelemetryRepository repository;

    // recebe o dto da controladora, converte para a entidade do banco e salva
    public TelemetryLog salvarTelemetria(TelemetryRequestDTO dto) {
        TelemetryLog log = new TelemetryLog();

        // repassa os dados recebidos pelo rádio
        log.setTemperaturaAmbiente(dto.getTemperaturaAmbiente());
        log.setTemperaturaSuperficial(dto.getTemperaturaSuperficial());
        log.setTimestampVoo(dto.getTimestampVoo());

        // o campo dataRecebimento sera preenchido automaticamente pela anotacao prepersist na entidade
        return repository.save(log);
    }

    public List<TelemetryLog> listarTodos() {
        return repository.findAll();
    }
}