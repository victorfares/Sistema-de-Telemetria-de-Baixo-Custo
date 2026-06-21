package com.example.demo.controller;

import com.example.demo.dto.TelemetryRequestDTO;
import com.example.demo.model.TelemetryLog;
import com.example.demo.service.TelemetryService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/telemetry")
public class TelemetryController {

    @Autowired
    private TelemetryService service;

    // recebe o pacote json do esp32 via post e envia para o service salvar
    @PostMapping
    public ResponseEntity<TelemetryLog> receberDados(@RequestBody TelemetryRequestDTO dto) {
        TelemetryLog logSalvo = service.salvarTelemetria(dto);
        // imprime no console do servidor para acompanhamento em tempo real
        System.out.println("pacote salvo: temp superficial = " + logSalvo.getTemperaturaSuperficial() + " c");
        return ResponseEntity.ok(logSalvo);
    }

    // fornece os dados salvos para o frontend desenhar os graficos via get
    @GetMapping
    public ResponseEntity<List<TelemetryLog>> listarDados() {
        return ResponseEntity.ok(service.listarTodos());
    }
}