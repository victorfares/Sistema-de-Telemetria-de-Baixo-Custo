package com.example.demo.dto;

import lombok.Data;

@Data
public class TelemetryRequestDTO {
    // molda exatamente os dados brutos que o json do esp32 vai enviar na requisicao post
    private Double temperaturaAmbiente;
    private Double temperaturaSuperficial;
    private Long timestampVoo;
}