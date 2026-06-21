package com.example.demo.model;

import jakarta.persistence.*;
import lombok.Data;
import lombok.NoArgsConstructor;
import java.time.LocalDateTime;

@Entity
@Table(name = "telemetry_logs")
@Data // gera os getters, setters e construtores via lombok
@NoArgsConstructor
public class TelemetryLog {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    // campos que espelham a struct enviada pelo esp32
    private Double temperaturaAmbiente;
    private Double temperaturaSuperficial;
    private Long timestampVoo;

    // data e hora local do servidor no momento do recebimento
    private LocalDateTime dataRecebimento;

    @PrePersist
    protected void onCreate() {
        // preenche automaticamente a data de recebimento antes de salvar no banco
        dataRecebimento = LocalDateTime.now();
    }
}