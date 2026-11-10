import React, { useState, useEffect } from 'react';
import { useLocation } from 'react-router-dom';
import requestService from '../../services/requestService';

const VehicleHealthHistory = ({ showAlert }) => {
    const [carId, setCarId] = useState('');
    const [history, setHistory] = useState([]);
    const [filters, setFilters] = useState({ status: '', start_date: '', end_date: '' });
    const [loading, setLoading] = useState(false);
    const [hasSearched, setHasSearched] = useState(false);
    
    const location = useLocation();

    const fetchHistory = async (idToSearch, currentFilters) => {
        if (!idToSearch) return;
        setLoading(true);
        setHasSearched(true);
        try {
            const result = await requestService.getVehicleHealthHistory(idToSearch, currentFilters);
            const historyData = Array.isArray(result) ? result : (result.data || []);
            setHistory(historyData);
        } catch (err) {
            console.error("Search Error:", err);
            setHistory([]);
            if (showAlert) showAlert("danger", "Could not retrieve history for this Vehicle ID.");
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        const queryParams = new URLSearchParams(location.search);
        const urlCarId = queryParams.get('car_id');
        
        if (urlCarId) {
            setCarId(urlCarId); 
            fetchHistory(urlCarId, filters);
        }
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [location.search]);

    const handleSearch = (e) => {
        e.preventDefault();
        fetchHistory(carId, filters);
    };

    return (
        <div className="vehicle-health-history">
            <div className="d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center pt-3 pb-2 mb-3 border-bottom">
                <h1 className="h2">Diagnostic History</h1>
            </div>
            
            <div className="card mb-4">
                <div className="card-body">
                    <form onSubmit={handleSearch} className="row g-3">
                        <div className="col-md-3">
                            <label className="form-label fw-bold small text-muted">VEHICLE ID</label>
                            <input 
                                className="form-control" 
                                value={carId} 
                                onChange={e => setCarId(e.target.value)} 
                                placeholder="e.g. mc180a" 
                            />
                        </div>
                        <div className="col-md-3">
                            <label className="form-label fw-bold small text-muted">HEALTH STATUS</label>
                            <select 
                                className="form-select" 
                                value={filters.status} 
                                onChange={e => setFilters({...filters, status: e.target.value})}
                            >
                                <option value="">All Logs</option>
                                <option value="Healthy">Healthy Only</option>
                                <option value="Critical">Critical Only</option>
                            </select>
                        </div>
                        <div className="col-md-3">
                            <label className="form-label fw-bold small text-muted">START DATE</label>
                            <input 
                                type="date" 
                                className="form-control" 
                                value={filters.start_date} 
                                onChange={e => setFilters({...filters, start_date: e.target.value})} 
                            />
                        </div>
                        <div className="col-md-3 d-flex align-items-end">
                            <button type="submit" className="btn btn-primary w-100" disabled={loading}>
                                <i className="bi bi-search me-2"></i> {loading ? "Analyzing..." : "Pull Reports"}
                            </button>
                        </div>
                    </form>
                </div>
            </div>

            <div className="row g-4">
                {history.length > 0 ? (
                    history.map((report, idx) => {
                        const isCritical = (report.overall_health || '').toUpperCase() === 'CRITICAL';
                        
                        return (
                            <div key={report.id || idx} className="col-md-6 col-xl-4">
                                <div className="card h-100 shadow-sm border-0">
                                    <div className={`card-header d-flex justify-content-between align-items-center text-white ${isCritical ? 'bg-danger' : 'bg-success'}`}>
                                        <span className="small fw-bold">
                                            <i className="bi bi-calendar3 me-1"></i> {new Date(report.timestamp).toLocaleString()}
                                        </span>
                                        <span className="badge bg-white text-dark">
                                            {report.overall_health ? report.overall_health.toUpperCase() : 'UNKNOWN'}
                                        </span>
                                    </div>

                                    <div className="card-body bg-light">
                                        {report.ecu_snapshots && report.ecu_snapshots.map((ecu, eIdx) => (
                                            <div key={eIdx} className="mb-3 border-bottom pb-2">
                                                <div className="d-flex align-items-center mb-2 fw-bold text-primary">
                                                    <i className="bi bi-cpu me-2"></i> {ecu.ecu_name.toUpperCase()}
                                                </div>
                                                
                                                <div className="row g-2">
                                                    {ecu.sensor_data && Object.entries(ecu.sensor_data).map(([key, value]) => (
                                                        <div key={key} className="col-6">
                                                            <div className="p-2 border rounded bg-white text-center">
                                                                <div className="text-muted small fw-bold text-uppercase" style={{ fontSize: '0.7rem' }}>
                                                                    {key}
                                                                </div>
                                                                <div className="fw-bold">
                                                                    {value}
                                                                </div>
                                                            </div>
                                                        </div>
                                                    ))}
                                                </div>
                                            </div>
                                        ))}
                                    </div>
                                </div>
                            </div>
                        );
                    })
                ) : hasSearched && !loading ? (
                    <div className="col-12">
                        <div className="alert alert-info text-center" role="alert">
                            No diagnostic records found for vehicle: <strong>{carId}</strong>
                        </div>
                    </div>
                ) : null}
            </div>
        </div>
    );
};

export default VehicleHealthHistory;